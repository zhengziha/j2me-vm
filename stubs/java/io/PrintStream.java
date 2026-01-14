package java.io;

public class PrintStream {
    public PrintStream() {
    }
    
    public void println(String s) {
        printlnNative(s);
    }
    
    public void println(int i) {
        printlnNative(String.valueOf(i));
    }
    
    public void println(boolean b) {
        printlnNative(b ? "true" : "false");
    }
    
    public void print(String s) {
        printNative(s);
    }
    
    public void print(int i) {
        printNative(String.valueOf(i));
    }
    
    private native void printlnNative(String s);
    private native void printNative(String s);
}
