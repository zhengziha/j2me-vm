package java.lang;

public class Double {
    public static final double POSITIVE_INFINITY = 1.0 / 0.0;
    public static final double NEGATIVE_INFINITY = -1.0 / 0.0;
    public static final double NaN = 0.0 / 0.0;
    public static final double MAX_VALUE = 1.7976931348623157e+308;
    public static final double MIN_VALUE = 4.9e-324;

    private double value;

    public Double(double value) {
        this.value = value;
    }

    public double doubleValue() {
        return value;
    }

    public float floatValue() {
        return (float)value;
    }

    public static native long doubleToLongBits(double value);
    public static native double longBitsToDouble(long bits);

    public String toString() {
        return toString(value);
    }
    
    public static String toString(double d) {
        return toStringNative(d);
    }
    
    private static native String toStringNative(double d);

    public boolean equals(Object obj) {
        return (obj instanceof Double) && (doubleToLongBits(((Double)obj).value) == doubleToLongBits(value));
    }

    public int hashCode() {
        long bits = doubleToLongBits(value);
        return (int)(bits ^ (bits >>> 32));
    }

    public static boolean isNaN(double v) {
        return (v != v);
    }

    public static boolean isInfinite(double v) {
        return (v == POSITIVE_INFINITY) || (v == NEGATIVE_INFINITY);
    }
}
