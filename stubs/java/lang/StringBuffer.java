package java.lang;
public class StringBuffer {
    public StringBuffer() { init(); }
    public StringBuffer(String str) { init(); append(str); }
    private native void init();
    public native StringBuffer append(String str);
    public StringBuffer append(Object obj) { return append(String.valueOf(obj)); }
    public StringBuffer append(int i) { return append(String.valueOf(i)); }
    public StringBuffer append(char c) { return append(String.valueOf(c)); }
    public native StringBuffer insert(int offset, String str);
    public native StringBuffer delete(int start, int end);
    public native String toString();
}
