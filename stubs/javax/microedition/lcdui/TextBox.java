package javax.microedition.lcdui;

public class TextBox extends Screen {
    private String text;
    private int maxSize;
    private int constraints;

    public TextBox(String title, String text, int maxSize, int constraints) {
        setTitle(title);
        this.text = (text == null) ? "" : text;
        this.maxSize = maxSize;
        this.constraints = constraints;
    }

    public String getString() {
        return text;
    }

    public void setString(String text) {
        this.text = (text == null) ? "" : text;
    }

    public int getChars(char[] data) {
        char[] chars = text.toCharArray();
        System.arraycopy(chars, 0, data, 0, chars.length);
        return chars.length;
    }

    public void setChars(char[] data, int offset, int length) {
        this.text = new String(data, offset, length);
    }

    public void insert(String src, int position) {
        StringBuffer sb = new StringBuffer(text);
        sb.insert(position, src);
        text = sb.toString();
    }
    
    public void insert(char[] data, int offset, int length, int position) {
        insert(new String(data, offset, length), position);
    }

    public void delete(int offset, int length) {
        StringBuffer sb = new StringBuffer(text);
        sb.delete(offset, offset + length);
        text = sb.toString();
    }

    public int getMaxSize() {
        return maxSize;
    }

    public int setMaxSize(int maxSize) {
        if (maxSize <= 0) throw new IllegalArgumentException();
        this.maxSize = maxSize;
        if (text.length() > maxSize) {
            text = text.substring(0, maxSize);
        }
        return maxSize;
    }

    public int getConstraints() {
        return constraints;
    }

    public void setConstraints(int constraints) {
        this.constraints = constraints;
    }

    public int getCaretPosition() {
        return 0; // Simplified
    }
}
