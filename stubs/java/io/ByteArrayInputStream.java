package java.io;

public class ByteArrayInputStream extends InputStream {
    protected byte buf[];
    protected int pos;
    protected int mark = 0;
    protected int count;

    public ByteArrayInputStream(byte buf[]) {
        this.buf = buf;
        this.pos = 0;
        this.count = buf.length;
    }

    public ByteArrayInputStream(byte buf[], int offset, int length) {
        this.buf = buf;
        this.pos = offset;
        this.count = Math.min(offset + length, buf.length);
        this.mark = offset;
    }

    public synchronized int read() {
        if (buf == null) {
            // Use native stream implementation
            return nativeRead();
        }
        return (pos < count) ? (buf[pos++] & 0xff) : -1;
    }

    public synchronized int read(byte b[], int off, int len) {
        if (b == null) {
            throw new NullPointerException();
        } else if (off < 0 || len < 0 || len > b.length - off) {
            throw new IndexOutOfBoundsException();
        }
        if (pos >= count) {
            return -1;
        }
        if (pos + len > count) {
            len = count - pos;
        }
        if (len <= 0) {
            return 0;
        }
        if (buf == null) {
            // Use native stream implementation
            return nativeRead(b, off, len);
        }
        System.arraycopy(buf, pos, b, off, len);
        pos += len;
        return len;
    }

    public synchronized long skip(long n) {
        if (buf == null) {
            // Use native stream implementation
            return nativeSkip(n);
        }
        if (pos + n > count) {
            n = count - pos;
        }
        if (n < 0) {
            return 0;
        }
        pos += n;
        return n;
    }

    public synchronized int available() {
        if (buf == null) {
            // Use native stream implementation
            return nativeAvailable();
        }
        return count - pos;
    }

    public boolean markSupported() {
        return true;
    }

    public void mark(int readlimit) {
        mark = pos;
        if (buf == null) {
            // Use native stream implementation
            nativeMark(readlimit);
        }
    }

    public synchronized void reset() {
        pos = mark;
        if (buf == null) {
            // Use native stream implementation
            nativeReset();
        }
    }

    public void close() throws IOException {
        if (buf == null) {
            // Use native stream implementation
            nativeClose();
        }
    }

    // Native methods for using NativeInputStream
    private native int nativeRead();
    private native int nativeRead(byte b[], int off, int len);
    private native long nativeSkip(long n);
    private native int nativeAvailable();
    private native void nativeMark(int readlimit);
    private native void nativeReset();
    private native void nativeClose();
}
