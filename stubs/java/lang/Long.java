package java.lang;

public class Long {
    public static final long MIN_VALUE = 0x8000000000000000L;
    public static final long MAX_VALUE = 0x7fffffffffffffffL;

    private long value;

    public Long(long value) {
        this.value = value;
    }

    public long longValue() {
        return value;
    }

    public int hashCode() {
        return (int)(value ^ (value >>> 32));
    }

    public boolean equals(Object obj) {
        if (obj instanceof Long) {
            return value == ((Long)obj).longValue();
        }
        return false;
    }

    public String toString() {
        return toString(value);
    }

    public static String toString(long i) {
        return "" + i; // Simplified
    }

    public static long parseLong(String s) throws NumberFormatException {
        return parseLong(s, 10);
    }

    public static long parseLong(String s, int radix) throws NumberFormatException {
        if (s == null) {
            throw new NumberFormatException("null");
        }
        // Simplified implementation, does not handle negatives properly or overflow
        long result = 0;
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            int digit = Character.digit(c, radix);
            if (digit == -1) throw new NumberFormatException(s);
            result = result * radix + digit;
        }
        return result;
    }
}
