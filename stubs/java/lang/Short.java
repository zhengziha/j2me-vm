package java.lang;

public class Short {
    public static final short MIN_VALUE = -32768;
    public static final short MAX_VALUE = 32767;

    private short value;

    public Short(short value) {
        this.value = value;
    }

    public short shortValue() {
        return value;
    }

    public int hashCode() {
        return (int)value;
    }

    public boolean equals(Object obj) {
        if (obj instanceof Short) {
            return value == ((Short)obj).shortValue();
        }
        return false;
    }

    public String toString() {
        return Integer.toString((int)value);
    }

    public static short parseShort(String s) throws NumberFormatException {
        return (short)Integer.parseInt(s);
    }
}
