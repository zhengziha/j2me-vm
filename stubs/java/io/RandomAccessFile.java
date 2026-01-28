package java.io;

public class RandomAccessFile implements DataInput, DataOutput {
    private static final int READ_ONLY = 1;
    private static final int READ_WRITE = 2;
    
    private int streamId;
    private int mode;
    private long filePointer;
    
    private native void openNative(String name, String mode);
    private native void closeNative();
    private native int readNative();
    private native int readNative(byte[] b, int off, int len);
    private native void writeNative(int b);
    private native void writeNative(byte[] b, int off, int len);
    private native void seekNative(long pos);
    private native long getFilePointerNative();
    private native long lengthNative();
    
    public RandomAccessFile(String name, String mode) {
        this.mode = parseMode(mode);
        this.filePointer = 0;
        openNative(name, mode);
    }
    
    private int parseMode(String mode) {
        if (mode.equals("r")) {
            return READ_ONLY;
        } else if (mode.equals("rw")) {
            return READ_WRITE;
        } else if (mode.equals("rws")) {
            return READ_WRITE;
        } else if (mode.equals("rwd")) {
            return READ_WRITE;
        }
        throw new IllegalArgumentException("Invalid mode: " + mode);
    }
    
    public void close() throws IOException {
        closeNative();
    }
    
    public int read() throws IOException {
        return readNative();
    }
    
    public int read(byte[] b) throws IOException {
        return read(b, 0, b.length);
    }
    
    public int read(byte[] b, int off, int len) throws IOException {
        return readNative(b, off, len);
    }
    
    public void readFully(byte[] b) throws IOException {
        readFully(b, 0, b.length);
    }
    
    public void readFully(byte[] b, int off, int len) throws IOException {
        if (len < 0) {
            throw new IndexOutOfBoundsException();
        }
        int n = 0;
        while (n < len) {
            int count = read(b, off + n, len - n);
            if (count < 0) {
                throw new EOFException();
            }
            n += count;
        }
    }
    
    public void write(int b) throws IOException {
        writeNative(b);
    }
    
    public void write(byte[] b) throws IOException {
        write(b, 0, b.length);
    }
    
    public void write(byte[] b, int off, int len) throws IOException {
        writeNative(b, off, len);
    }
    
    public void seek(long pos) throws IOException {
        seekNative(pos);
        filePointer = pos;
    }
    
    public long getFilePointer() throws IOException {
        return getFilePointerNative();
    }
    
    public long length() throws IOException {
        return lengthNative();
    }
    
    public void setLength(long newLength) throws IOException {
        throw new IOException("setLength not supported");
    }
    
    public int skipBytes(int n) throws IOException {
        long pos = getFilePointer();
        seek(pos + n);
        return n;
    }
    
    public boolean readBoolean() throws IOException {
        int ch = this.read();
        if (ch < 0) {
            throw new EOFException();
        }
        return (ch != 0);
    }
    
    public byte readByte() throws IOException {
        int ch = this.read();
        if (ch < 0) {
            throw new EOFException();
        }
        return (byte)ch;
    }
    
    public int readUnsignedByte() throws IOException {
        int ch = this.read();
        if (ch < 0) {
            throw new EOFException();
        }
        return ch;
    }
    
    public short readShort() throws IOException {
        int ch1 = this.read();
        int ch2 = this.read();
        if ((ch1 | ch2) < 0) {
            throw new EOFException();
        }
        return (short)((ch1 << 8) + (ch2 << 0));
    }
    
    public int readUnsignedShort() throws IOException {
        int ch1 = this.read();
        int ch2 = this.read();
        if ((ch1 | ch2) < 0) {
            throw new EOFException();
        }
        return (ch1 << 8) + (ch2 << 0);
    }
    
    public char readChar() throws IOException {
        int ch1 = this.read();
        int ch2 = this.read();
        if ((ch1 | ch2) < 0) {
            throw new EOFException();
        }
        return (char)((ch1 << 8) + (ch2 << 0));
    }
    
    public int readInt() throws IOException {
        int ch1 = this.read();
        int ch2 = this.read();
        int ch3 = this.read();
        int ch4 = this.read();
        if ((ch1 | ch2 | ch3 | ch4) < 0) {
            throw new EOFException();
        }
        return ((ch1 << 24) + (ch2 << 16) + (ch3 << 8) + (ch4 << 0));
    }
    
    public long readLong() throws IOException {
        return ((long)readInt() << 32) + (readInt() & 0xFFFFFFFFL);
    }
    
    public float readFloat() throws IOException {
        return Float.intBitsToFloat(readInt());
    }
    
    public double readDouble() throws IOException {
        return Double.longBitsToDouble(readLong());
    }
    
    public String readLine() throws IOException {
        StringBuffer input = new StringBuffer();
        int c = -1;
        boolean eol = false;
        
        while (!eol) {
            switch (c = read()) {
                case -1:
                case '\n':
                    eol = true;
                    break;
                case '\r':
                    eol = true;
                    long cur = getFilePointer();
                    if ((read()) != '\n') {
                        seek(cur);
                    }
                    break;
                default:
                    input.append((char)c);
                    break;
            }
        }
        
        if ((c == -1) && (input.toString().length() == 0)) {
            return null;
        }
        return input.toString();
    }
    
    public String readUTF() throws IOException {
        return DataInputStream.readUTF(this);
    }
    
    public void writeBoolean(boolean v) throws IOException {
        write(v ? 1 : 0);
    }
    
    public void writeByte(int v) throws IOException {
        write(v);
    }
    
    public void writeShort(int v) throws IOException {
        write((v >>> 8) & 0xFF);
        write((v >>> 0) & 0xFF);
    }
    
    public void writeChar(int v) throws IOException {
        write((v >>> 8) & 0xFF);
        write((v >>> 0) & 0xFF);
    }
    
    public void writeInt(int v) throws IOException {
        write((v >>> 24) & 0xFF);
        write((v >>> 16) & 0xFF);
        write((v >>>  8) & 0xFF);
        write((v >>>  0) & 0xFF);
    }
    
    public void writeLong(long v) throws IOException {
        write((int)(v >>> 56) & 0xFF);
        write((int)(v >>> 48) & 0xFF);
        write((int)(v >>> 40) & 0xFF);
        write((int)(v >>> 32) & 0xFF);
        write((int)(v >>> 24) & 0xFF);
        write((int)(v >>> 16) & 0xFF);
        write((int)(v >>>  8) & 0xFF);
        write((int)(v >>>  0) & 0xFF);
    }
    
    public void writeFloat(float v) throws IOException {
        writeInt(Float.floatToIntBits(v));
    }
    
    public void writeDouble(double v) throws IOException {
        writeLong(Double.doubleToLongBits(v));
    }
    
    public void writeBytes(String s) throws IOException {
        int len = s.length();
        for (int i = 0; i < len; i++) {
            write((byte)s.charAt(i));
        }
    }
    
    public void writeChars(String s) throws IOException {
        int clen = s.length();
        for (int i = 0; i < clen; i++) {
            int v = s.charAt(i);
            write((v >>> 8) & 0xFF);
            write((v >>> 0) & 0xFF);
        }
    }
    
    public void writeUTF(String str) throws IOException {
        int strlen = str.length();
        int utflen = 0;
        int c, count = 0;

        for (int i = 0; i < strlen; i++) {
            c = str.charAt(i);
            if ((c >= 0x0001) && (c <= 0x007F)) {
                utflen++;
            } else if (c > 0x07FF) {
                utflen += 3;
            } else {
                utflen += 2;
            }
        }

        if (utflen > 65535)
            throw new UTFDataFormatException("encoded string too long: " + utflen + " bytes");

        byte[] bytearr = new byte[utflen+2];

        bytearr[count++] = (byte) ((utflen >>> 8) & 0xFF);
        bytearr[count++] = (byte) ((utflen >>> 0) & 0xFF);

        int i=0;
        for (i=0; i<strlen; i++) {
           c = str.charAt(i);
           if (!((c >= 0x0001) && (c <= 0x007F))) break;
           bytearr[count++] = (byte) c;
        }

        for (;i < strlen; i++){
            c = str.charAt(i);
            if ((c >= 0x0001) && (c <= 0x007F)) {
                bytearr[count++] = (byte) c;

            } else if (c > 0x07FF) {
                bytearr[count++] = (byte) (0xE0 | ((c >> 12) & 0x0F));
                bytearr[count++] = (byte) (0x80 | ((c >>  6) & 0x3F));
                bytearr[count++] = (byte) (0x80 | ((c >>  0) & 0x3F));
            } else {
                bytearr[count++] = (byte) (0xC0 | ((c >>  6) & 0x1F));
                bytearr[count++] = (byte) (0x80 | ((c >>  0) & 0x3F));
            }
        }
        write(bytearr, 0, utflen+2);
    }
}
