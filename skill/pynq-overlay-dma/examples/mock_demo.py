"""CPU-only demonstration using fake buffers and a fake DMA; no PYNQ required."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from pynq_examples import SimpleDmaSession


class Buffer:
    def __init__(self): self.nbytes = 16
    def flush(self): print('MOCK flush buffer')
    def invalidate(self): print('MOCK invalidate output')


class Channel:
    error = False
    def __init__(self, name): self.name, self.transferred = name, 0
    def transfer(self, buffer):
        self.transferred = buffer.nbytes
        print('MOCK transfer', self.name, buffer.nbytes)
    def wait(self): print('MOCK wait', self.name)


class Dma:
    def __init__(self):
        self.recvchannel = Channel('recv')
        self.sendchannel = Channel('send')


if __name__ == '__main__':
    session = SimpleDmaSession(Dma(), Buffer(), Buffer())
    session.run()
    print('MOCK completed; no FPGA transaction or numerical output tested')
