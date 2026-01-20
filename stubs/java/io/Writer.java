package java.io;

public abstract class Writer {
    protected Object lock;

    protected Writer() {
        this.lock = this;
    }

    protected Writer(Object lock) {
        if (lock == null) {
            throw new NullPointerException();
        }
        this.lock = lock;
    }

    public void write(int c) throws IOException {
        synchronized (lock) {
            char cb[] = new char[1];
            cb[0] = (char) c;
            write(cb, 0, 1);
        }
    }

    public void write(char cbuf[]) throws IOException {
        write(cbuf, 0, cbuf.length);
    }

    public abstract void write(char cbuf[], int off, int len) throws IOException;

    public void write(String str) throws IOException {
        write(str, 0, str.length());
    }

    public void write(String str, int off, int len) throws IOException {
        synchronized (lock) {
            char cbuf[];
            if (len <= 1024) {
                if (writeBuffer == null) {
                    writeBuffer = new char[1024];
                }
                cbuf = writeBuffer;
            } else {
                cbuf = new char[len];
            }
            int c_off = 0;
            while (c_off < len) {
                // In J2ME we don't have getChars, so we copy manually
                for (int i=0; i<len; i++) {
                     cbuf[i] = str.charAt(off + i);
                }
                write(cbuf, 0, len);
                c_off += len;
            }
        }
    }

    public abstract void flush() throws IOException;

    public abstract void close() throws IOException;

    private char writeBuffer[];
}
