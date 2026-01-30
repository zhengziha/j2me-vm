package java.io;

public class ResourceInputStream extends InputStream {
    private int streamId;
    private int pos;
    private int mark = 0;
    private int count;

    public ResourceInputStream(int streamId, int size) {
        this.streamId = streamId;
        this.pos = 0;
        this.count = size;
    }

    @Override
    public int read() throws IOException {
        if (pos >= count) {
            return -1;
        }
        int result = nativeRead();
        if (result != -1) {
            pos++;
        }
        return result;
    }

    @Override
    public int read(byte b[], int off, int len) throws IOException {
        if (b == null) {
            throw new NullPointerException();
        } else if (off < 0 || len < 0 || len > b.length - off) {
            throw new IndexOutOfBoundsException();
        }
        if (pos >= count) {
            return -1;
        }
        if (len <= 0) {
            return 0;
        }
        int bytesRead = nativeRead(b, off, len);
        if (bytesRead > 0) {
            pos += bytesRead;
        }
        return bytesRead;
    }

    @Override
    public long skip(long n) throws IOException {
        if (pos + n > count) {
            n = count - pos;
        }
        if (n < 0) {
            return 0;
        }
        long skipped = nativeSkip(n);
        pos += skipped;
        return skipped;
    }

    @Override
    public int available() throws IOException {
        return count - pos;
    }

    @Override
    public void close() throws IOException {
        nativeClose();
    }

    @Override
    public void mark(int readlimit) {
        mark = pos;
        nativeMark(readlimit);
    }

    @Override
    public void reset() throws IOException {
        pos = mark;
        nativeReset();
    }

    @Override
    public boolean markSupported() {
        return true;
    }

    // Native methods for using NativeInputStream
    private native int nativeRead();
    private native int nativeRead(byte b[], int off, int len);
    private native long nativeSkip(long n);
    private native void nativeMark(int readlimit);
    private native void nativeReset();
    private native void nativeClose();
}
