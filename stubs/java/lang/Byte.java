package java.lang;

public class Byte {
    public static final byte MIN_VALUE = -128;
    public static final byte MAX_VALUE = 127;

    private byte value;

    public Byte(byte value) {
        this.value = value;
    }

    public byte byteValue() {
        return value;
    }

    public int hashCode() {
        return (int)value;
    }

    public boolean equals(Object obj) {
        if (obj instanceof Byte) {
            return value == ((Byte)obj).byteValue();
        }
        return false;
    }

    public String toString() {
        return Integer.toString((int)value);
    }

    public static byte parseByte(String s) throws NumberFormatException {
        return (byte)Integer.parseInt(s);
    }
}
