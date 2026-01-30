package java.io;

public class StringWriter extends Writer {
    private StringBuffer buf;

    public StringWriter() {
        buf = new StringBuffer();
        lock = buf;
    }

    public StringWriter(int initialSize) {
        if (initialSize < 0) {
            throw new IllegalArgumentException("Negative buffer size");
        }
        // StringBuffer stub doesn't support initial size, just use default
        buf = new StringBuffer();
        lock = buf;
    }

    public void write(int c) {
        buf.append((char) c);
    }

    public void write(char cbuf[], int off, int len) {
        if ((off < 0) || (off > cbuf.length) || (len < 0) ||
            ((off + len) > cbuf.length) || ((off + len) < 0)) {
            throw new IndexOutOfBoundsException();
        } else if (len == 0) {
            return;
        }
        // StringBuffer stub lacks append(char[], int, int), use String
        buf.append(new String(cbuf, off, len));
    }

    public void write(String str) {
        buf.append(str);
    }

    public void write(String str, int off, int len) {
        buf.append(str.substring(off, off + len));
    }

    public String toString() {
        return buf.toString();
    }

    public StringBuffer getBuffer() {
        return buf;
    }

    public void flush() {
    }

    public void close() throws IOException {
    }
}
