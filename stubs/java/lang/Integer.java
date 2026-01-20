package java.lang;

public class Integer {
    public static final int MIN_VALUE = 0x80000000;
    public static final int MAX_VALUE = 0x7fffffff;

    private int value;

    public Integer(int value) {
        this.value = value;
    }

    public int intValue() {
        return value;
    }

    public int hashCode() {
        return value;
    }

    public boolean equals(Object obj) {
        if (obj instanceof Integer) {
            return value == ((Integer)obj).intValue();
        }
        return false;
    }

    public String toString() {
        return toString(value);
    }

    public static String toString(int i) {
        return "" + i; // Simplified
    }
    
    public static String toString(int i, int radix) {
         return "" + i; // Simplified
    }

    public static String toHexString(int i) {
        return toString(i, 16);
    }
    
    public static String toBinaryString(int i) {
        return toString(i, 2);
    }

    public static int parseInt(String s) throws NumberFormatException {
        return parseInt(s, 10);
    }

    public static int parseInt(String s, int radix) throws NumberFormatException {
        if (s == null) {
            throw new NumberFormatException("null");
        }
        // Simplified implementation
        int result = 0;
        boolean negative = false;
        int i = 0, len = s.length();
        if (len > 0) {
            char firstChar = s.charAt(0);
            if (firstChar < '0') { // Possible leading "+" or "-"
                if (firstChar == '-') {
                    negative = true;
                } else if (firstChar != '+') {
                    throw new NumberFormatException(s);
                }
                i++;
            }
            while (i < len) {
                int digit = Character.digit(s.charAt(i++), radix);
                if (digit < 0) {
                    throw new NumberFormatException(s);
                }
                result *= radix;
                result += digit;
            }
        } else {
             throw new NumberFormatException(s);
        }
        return negative ? -result : result;
    }
    
    public static Integer valueOf(String s) throws NumberFormatException {
        return new Integer(parseInt(s));
    }
    
    public static Integer valueOf(int i) {
        return new Integer(i);
    }
}
