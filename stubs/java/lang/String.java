package java.lang;
public class String {
    public static String valueOf(Object obj) { return (obj == null) ? "null" : obj.toString(); }
    public static native String valueOf(int i);
}
