import struct
import zlib

def make_png(width, height):
    # Signature
    png = b'\x89PNG\r\n\x1a\n'
    
    # IHDR
    ihdr = struct.pack('!I4sIIBBBBB', 13, b'IHDR', width, height, 8, 2, 0, 0, 0)
    ihdr += struct.pack('!I', zlib.crc32(ihdr[4:]) & 0xFFFFFFFF)
    png += ihdr
    
    # IDAT
    # RGB data: Red (255, 0, 0)
    raw_data = b''
    for y in range(height):
        raw_data += b'\x00' # Filter type 0 (None)
        for x in range(width):
            raw_data += b'\xFF\x00\x00' # RGB
            
    compressed = zlib.compress(raw_data)
    idat = struct.pack('!I4s', len(compressed), b'IDAT') + compressed
    idat += struct.pack('!I', zlib.crc32(idat[4:]) & 0xFFFFFFFF)
    png += idat
    
    # IEND
    iend = struct.pack('!I4s', 0, b'IEND')
    iend += struct.pack('!I', zlib.crc32(iend[4:]) & 0xFFFFFFFF)
    png += iend
    
    return png

with open('test_image.png', 'wb') as f:
    f.write(make_png(50, 50))
