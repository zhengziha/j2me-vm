package java.io;

public class InputStreamReader extends Reader {
    private final InputStream in;

    public InputStreamReader(InputStream in) {
        super(in);
        this.in = in;
    }

    public InputStreamReader(InputStream in, String enc) throws UnsupportedEncodingException {
        super(in);
        if (enc == null)
            throw new NullPointerException("encoding is null");
        this.in = in;
    }

    public int read(char cbuf[], int off, int len) throws IOException {
        // Very simplified ASCII/UTF-8 reader
        byte[] buffer = new byte[len];
        int read = in.read(buffer);
        if (read == -1) return -1;
        for (int i = 0; i < read; i++) {
            cbuf[off + i] = (char) (buffer[i] & 0xFF);
        }
        return read;
    }

    public void close() throws IOException {
        in.close();
    }
}
