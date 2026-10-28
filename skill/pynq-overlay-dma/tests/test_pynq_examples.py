import hashlib
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from pynq_examples import SimpleDmaSession, download_overlay, prepare_overlay, read32, write32


class Overlay:
    def __init__(self, path, download):
        self.path, self.auto_download = path, download
        self.ip_dict = {'dma_0': {}}
        self.downloads = 0

    def download(self):
        self.downloads += 1


class Buffer:
    def __init__(self, nbytes, events):
        self.nbytes, self.events = nbytes, events

    def flush(self):
        self.events.append('flush')

    def invalidate(self):
        self.events.append('invalidate')


class Channel:
    def __init__(self, name, events, fail=None):
        self.name, self.events, self.fail = name, events, fail
        self.error = False
        self.transferred = 0

    def transfer(self, buffer):
        self.events.append(self.name + '.transfer')
        self.transferred = buffer.nbytes
        if self.fail == 'transfer':
            raise RuntimeError('transfer failed')

    def wait(self):
        self.events.append(self.name + '.wait')
        if self.fail == 'wait':
            raise RuntimeError('wait failed')
        if self.fail == 'interrupt':
            raise KeyboardInterrupt()
        if self.fail == 'short':
            self.transferred -= 1


class Dma:
    def __init__(self, events):
        self.sendchannel = Channel('send', events)
        self.recvchannel = Channel('recv', events)


class PynqSkillTest(unittest.TestCase):
    def test_overlay_integrity_and_explicit_download(self):
        with tempfile.TemporaryDirectory() as d:
            bit, hwh = Path(d) / 'demo.bit', Path(d) / 'demo.hwh'
            bit.write_bytes(b'bit'); hwh.write_bytes(b'hwh')
            digest = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
            ov = prepare_overlay(bit, hwh, digest(bit), digest(hwh), ['dma_0'], Overlay)
            self.assertFalse(ov.auto_download)
            with self.assertRaises(PermissionError):
                download_overlay(ov)
            self.assertEqual(ov.downloads, 0)
            download_overlay(ov, approved=True)
            self.assertEqual(ov.downloads, 1)
            with self.assertRaisesRegex(ValueError, 'SHA256'):
                prepare_overlay(bit, hwh, '0' * 64, digest(hwh), ['dma_0'], Overlay)
            with self.assertRaisesRegex(ValueError, 'missing IP'):
                prepare_overlay(bit, hwh, digest(bit), digest(hwh), ['missing'], Overlay)
            with self.assertRaisesRegex(ValueError, 'nonempty list'):
                prepare_overlay(bit, hwh, digest(bit), digest(hwh), 'dma_0', Overlay)

    def test_register_access_allowlist(self):
        class Ip:
            def __init__(self): self.writes = []
            def read(self, offset): return 17
            def write(self, offset, value): self.writes.append((offset, value))
        ip = Ip()
        self.assertEqual(read32(ip, 4, {4}), 17)
        with self.assertRaises(ValueError): read32(ip, 3, {3})
        with self.assertRaises(PermissionError): write32(ip, 4, 1, {4})
        self.assertEqual(ip.writes, [])
        write32(ip, 4, 1, {4}, approved=True)
        self.assertEqual(ip.writes, [(4, 1)])

    def test_dma_sequence_cache_and_reuse(self):
        events = []; dma = Dma(events); inp = Buffer(16, events); out = Buffer(16, events)
        session = SimpleDmaSession(dma, inp, out)
        self.assertIs(session.run(), out)
        self.assertEqual(events, ['flush', 'flush', 'recv.transfer', 'send.transfer', 'send.wait', 'recv.wait', 'invalidate'])
        self.assertEqual(session.state, 'ready')
        session.run()
        self.assertEqual(events.count('invalidate'), 2)

    def test_dma_failures_do_not_read_or_reuse(self):
        for channel, stage in [('recvchannel', 'transfer'), ('sendchannel', 'transfer'), ('sendchannel', 'wait'), ('recvchannel', 'wait')]:
            with self.subTest(channel=channel, stage=stage):
                events = []; dma = Dma(events); inp = Buffer(16, events); out = Buffer(16, events)
                getattr(dma, channel).fail = stage
                session = SimpleDmaSession(dma, inp, out)
                with self.assertRaises(RuntimeError): session.run()
                self.assertEqual(session.state, 'faulted')
                self.assertIs(session.input_buffer, inp)
                self.assertIs(session.output_buffer, out)
                self.assertNotIn('invalidate', events)
                with self.assertRaises(RuntimeError): session.run()

    def test_short_transfer_and_interrupt_are_faults(self):
        for failure, error in [('short', RuntimeError), ('interrupt', KeyboardInterrupt)]:
            events = []; dma = Dma(events); inp = Buffer(16, events); out = Buffer(16, events)
            dma.recvchannel.fail = failure
            session = SimpleDmaSession(dma, inp, out)
            with self.assertRaises(error): session.run()
            self.assertEqual(session.state, 'faulted')
            self.assertIs(session.output_buffer, out)
            self.assertNotIn('invalidate', events)

    def test_dma_error_or_invalid_buffers(self):
        events = []; dma = Dma(events); inp = Buffer(16, events); out = Buffer(16, events)
        with self.assertRaises(ValueError): SimpleDmaSession(dma, inp, inp)
        with self.assertRaises(ValueError): SimpleDmaSession(dma, Buffer(0, events), out)
        dma.recvchannel.error = True
        session = SimpleDmaSession(dma, inp, out)
        with self.assertRaises(RuntimeError): session.run()
        self.assertEqual(session.state, 'faulted')
        self.assertNotIn('invalidate', events)


if __name__ == '__main__':
    unittest.main()
