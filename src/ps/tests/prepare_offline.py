import pathlib
import sys
from PIL import Image

source = pathlib.Path(sys.argv[1])
target = pathlib.Path(sys.argv[2])
with Image.open(source) as image:
    print(f"Source JPEG size={image.size}; offline comparison fixture resized to 640x360")
    rgb = image.convert("RGB").resize((640, 360), Image.Resampling.BICUBIC).tobytes()
target.parent.mkdir(parents=True, exist_ok=True)
target.write_bytes(rgb)
print(f"Prepared {target}: {len(rgb)} RGB888 bytes")
