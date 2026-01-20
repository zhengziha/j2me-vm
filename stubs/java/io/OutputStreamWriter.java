package java.io;

public class OutputStreamWriter extends Writer {
    private final OutputStream out;

    public OutputStreamWriter(OutputStream out) {
        super(out);
        this.out = out;
    }

    public OutputStreamWriter(OutputStream out, String enc) throws UnsupportedEncodingException {
        super(out);
        if (enc == null)
            throw new NullPointerException("encoding is null");
        this.out = out;
    }

    public void write(char cbuf[], int off, int len) throws IOException {
        // Very simplified ASCII/UTF-8 writer
        for (int i = 0; i < len; i++) {
            out.write(cbuf[off + i]);
        }
    }

    public void flush() throws IOException {
        out.flush();
    }

    public void close() throws IOException {
        out.close();
    }
}
