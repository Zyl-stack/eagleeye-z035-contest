"""PYNQ examples for an independently approved Overlay and simple AXI DMA.

No hardware is accessed on import. See readme.md before calling any function.
"""
from hashlib import sha256
from pathlib import Path


def _sha256(path):
    h = sha256()
    with Path(path).open('rb') as f:
        for block in iter(lambda: f.read(1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()


def prepare_overlay(bit, hwh, bit_sha256, hwh_sha256, required_ips, overlay_factory=None):
    """Check local identities, then parse an Overlay without downloading it."""
    bit, hwh = Path(bit), Path(hwh)
    if bit.suffix.lower() != '.bit' or hwh.suffix.lower() != '.hwh' or bit.with_suffix('.hwh') != hwh:
        raise ValueError('BIT and HWH must share the same directory and basename')
    if not isinstance(required_ips, (list, tuple)) or not required_ips or not all(isinstance(x, str) and x for x in required_ips):
        raise ValueError('provide a nonempty list of required IP instance names')
    if _sha256(bit) != bit_sha256.lower() or _sha256(hwh) != hwh_sha256.lower():
        raise ValueError('BIT/HWH SHA256 mismatch')
    if overlay_factory is None:
        from pynq import Overlay
        overlay_factory = Overlay
    overlay = overlay_factory(str(bit), download=False)
    missing = sorted(set(required_ips) - set(overlay.ip_dict))
    if missing:
        raise ValueError('missing IP instances: ' + ', '.join(missing))
    return overlay


def download_overlay(overlay, *, approved=False):
    """Reconfigure PL only when the caller explicitly confirms local approval."""
    if not approved:
        raise PermissionError('Overlay download not approved')
    overlay.download()


def read32(ip, offset, allowed_offsets):
    """Read only explicitly approved 32-bit registers (reads can have side effects)."""
    if not isinstance(offset, int) or offset < 0 or offset % 4 or offset not in allowed_offsets:
        raise ValueError('offset not in approved aligned 32-bit register list')
    return ip.read(offset)


def write32(ip, offset, value, allowed_offsets, *, approved=False):
    """Write only after explicit approval and project-specific register review."""
    if not approved:
        raise PermissionError('register write not approved')
    if not isinstance(offset, int) or offset < 0 or offset % 4 or offset not in allowed_offsets:
        raise ValueError('offset not in approved aligned 32-bit register list')
    if not isinstance(value, int) or not 0 <= value <= 0xffffffff:
        raise ValueError('value must fit in 32 bits')
    ip.write(offset, value)


class SimpleDmaSession:
    """Simple-mode, single-owner example; never silently retry a failed transfer."""
    def __init__(self, dma, input_buffer, output_buffer):
        if input_buffer is output_buffer:
            raise ValueError('input and output must be distinct buffers')
        if input_buffer.nbytes <= 0 or output_buffer.nbytes <= 0:
            raise ValueError('buffers must have nonzero size')
        self.dma = dma
        self.input_buffer = input_buffer
        self.output_buffer = output_buffer
        self.state = 'ready'

    def run(self):
        if self.state != 'ready':
            raise RuntimeError('session is not ready; do not reuse after failure')
        self.state = 'active'
        try:
            self.input_buffer.flush()
            self.output_buffer.flush()
            self.dma.recvchannel.transfer(self.output_buffer)
            self.dma.sendchannel.transfer(self.input_buffer)
            # Version-dependent: PYNQ wait() may block indefinitely. See README.
            self.dma.sendchannel.wait()
            self.dma.recvchannel.wait()
            if self.dma.sendchannel.error or self.dma.recvchannel.error:
                raise RuntimeError('DMA channel reports an error')
            if (self.dma.sendchannel.transferred != self.input_buffer.nbytes or
                    self.dma.recvchannel.transferred != self.output_buffer.nbytes):
                raise RuntimeError('DMA transferred length does not match contract')
            self.output_buffer.invalidate()
        except BaseException:
            self.state = 'faulted'
            raise
        self.state = 'ready'
        return self.output_buffer
