package java.lang;
public class String {
    private byte[] value;
    private int offset;
    private int count;
    
    public static String valueOf(Object obj) { return (obj == null) ? "null" : obj.toString(); }
    public static native String valueOf(int i);
    public native byte[] getBytes();
    public String(byte[] data) { }
    
    public int length() {
        return count;
    }
}
