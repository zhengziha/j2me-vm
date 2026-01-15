package java.io;

public class PrintStream {
    private native void printlnNative(String s);
    
    public void println(String s) {
        printlnNative(s);
    }
    
    public void println(int i) {
        printlnNative(String.valueOf(i));
    }
    
    public void println(long l) {
        printlnNative(String.valueOf(l));
    }
    
    public void println(float f) {
        printlnNative(String.valueOf(f));
    }
    
    public void println(double d) {
        printlnNative(String.valueOf(d));
    }
    
    public void println(char c) {
        printlnNative(String.valueOf(c));
    }
    
    public void println(boolean b) {
        printlnNative(String.valueOf(b));
    }
}
