package java.lang;

public class Character {
    public static final char MIN_VALUE = '\u0000';
    public static final char MAX_VALUE = '\uffff';
    public static final int MIN_RADIX = 2;
    public static final int MAX_RADIX = 36;
    public static final int MIN_SUPPLEMENTARY_CODE_POINT = 0x010000;

    private char value;

    public Character(char value) {
        this.value = value;
    }

    public char charValue() {
        return value;
    }

    public int hashCode() {
        return (int)value;
    }

    public boolean equals(Object obj) {
        if (obj instanceof Character) {
            return value == ((Character)obj).charValue();
        }
        return false;
    }

    public String toString() {
        char[] buf = {value};
        return new String(buf);
    }

    public static boolean isDigit(char ch) {
        return ch >= '0' && ch <= '9';
    }

    public static boolean isLowerCase(char ch) {
        return ch >= 'a' && ch <= 'z';
    }

    public static boolean isUpperCase(char ch) {
        return ch >= 'A' && ch <= 'Z';
    }
    
    public static char toLowerCase(char ch) {
        if (isUpperCase(ch)) {
            return (char)(ch + ('a' - 'A'));
        }
        return ch;
    }

    public static char toUpperCase(char ch) {
        if (isLowerCase(ch)) {
            return (char)(ch - ('a' - 'A'));
        }
        return ch;
    }

    public static int digit(char ch, int radix) {
        if (radix < MIN_RADIX || radix > MAX_RADIX) {
            return -1;
        }
        if (ch >= '0' && ch <= '9') {
            return (ch - '0' < radix) ? (ch - '0') : -1;
        }
        if (ch >= 'a' && ch <= 'z') {
            return (ch - 'a' + 10 < radix) ? (ch - 'a' + 10) : -1;
        }
        if (ch >= 'A' && ch <= 'Z') {
            return (ch - 'A' + 10 < radix) ? (ch - 'A' + 10) : -1;
        }
        return -1;
    }
}
