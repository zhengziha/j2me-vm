package java.io;

public class ByteArrayInputStream extends InputStream {
    private byte[] buf;
    private int pos;
    private int count;

    public ByteArrayInputStream(byte[] buf) {
        this.buf = buf;
        this.pos = 0;
        this.count = buf.length;
    }

    public ByteArrayInputStream(byte[] buf, int offset, int length) {
        this.buf = buf;
        this.pos = offset;
        this.count = Math.min(offset + length, buf.length);
    }

    public int read() {
        return (pos < count) ? (buf[pos++] & 0xff) : -1;
    }

    public int read(byte[] b, int off, int len) {
        if (pos >= count) {
            return -1;
        }
        int avail = count - pos;
        if (len > avail) {
            len = avail;
        }
        if (len <= 0) {
            return 0;
        }
        System.arraycopy(buf, pos, b, off, len);
        pos += len;
        return len;
    }

    public long skip(long n) {
        if (pos >= count) {
            return 0;
        }
        int avail = count - pos;
        if (n > avail) {
            n = avail;
        }
        pos += n;
        return n;
    }

    public int available() {
        return count - pos;
    }

    public void reset() {
        pos = 0;
    }
}
