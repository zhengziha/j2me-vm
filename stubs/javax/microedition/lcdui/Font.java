package javax.microedition.lcdui;

public class Font {
    public static final int FACE_SYSTEM = 0;
    public static final int FACE_MONOSPACE = 32;
    public static final int FACE_PROPORTIONAL = 64;

    public static final int STYLE_PLAIN = 0;
    public static final int STYLE_BOLD = 1;
    public static final int STYLE_ITALIC = 2;
    public static final int STYLE_UNDERLINED = 4;

    public static final int SIZE_SMALL = 8;
    public static final int SIZE_MEDIUM = 0;
    public static final int SIZE_LARGE = 16;

    private int face;
    private int style;
    private int size;

    private Font(int face, int style, int size) {
        this.face = face;
        this.style = style;
        this.size = size;
    }

    public static Font getDefaultFont() {
        return getFont(FACE_SYSTEM, STYLE_PLAIN, SIZE_MEDIUM);
    }

    public static Font getFont(int face, int style, int size) {
        return new Font(face, style, size);
    }

    public static Font getFont(int fontSpecifier) {
        return getDefaultFont();
    }

    public int getFace() {
        return face;
    }

    public int getStyle() {
        return style;
    }

    public int getSize() {
        return size;
    }

    public int getHeight() {
        return size == SIZE_SMALL ? 12 : (size == SIZE_LARGE ? 22 : 16);
    }

    public int getBaselinePosition() {
        return getHeight() - 2;
    }

    public int charWidth(char ch) {
        return stringWidth(String.valueOf(ch));
    }

    public int charsWidth(char[] ch, int offset, int length) {
        int width = 0;
        for (int i = offset; i < offset + length; i++) {
            width += charWidth(ch[i]);
        }
        return width;
    }

    public int stringWidth(String str) {
        return str.length() * (size == SIZE_SMALL ? 7 : (size == SIZE_LARGE ? 13 : 9));
    }

    public boolean isBold() {
        return (style & STYLE_BOLD) != 0;
    }

    public boolean isItalic() {
        return (style & STYLE_ITALIC) != 0;
    }

    public boolean isUnderlined() {
        return (style & STYLE_UNDERLINED) != 0;
    }
}
