package java.io;

public class PrintStream extends OutputStream {
    public PrintStream(OutputStream out) {
    }

    public void print(boolean b) { write(String.valueOf(b)); }
    public void print(char c) { write(String.valueOf(c)); }
    public void print(int i) { write(String.valueOf(i)); }
    public void print(long l) { write(String.valueOf(l)); }
    public void print(float f) { write(String.valueOf(f)); }
    public void print(double d) { write(String.valueOf(d)); }
    public void print(char s[]) { write(new String(s)); }
    public void print(String s) { write(s == null ? "null" : s); }
    public void print(Object obj) { write(String.valueOf(obj)); }

    public void println() {
        write("\n");
    }

    public void println(boolean x) {
        print(x);
        println();
    }

    public void println(char x) {
        print(x);
        println();
    }

    public void println(int x) {
        print(x);
        println();
    }

    public void println(long x) {
        print(x);
        println();
    }

    public void println(float x) {
        print(x);
        println();
    }

    public void println(double x) {
        print(x);
        println();
    }

    public void println(char x[]) {
        print(x);
        println();
    }

    public void println(String x) {
        printlnNative(x == null ? "null" : x);
    }

    public void println(Object x) {
        print(x);
        println();
    }
    
    public void write(int b) {
        // Native binding usually handles stdout
        printNative(String.valueOf((char)b));
    }
    
    public void write(byte buf[], int off, int len) {
        // Simple implementation
        printNative(new String(buf, off, len));
    }
    
    private void write(String s) {
        printNative(s);
    }
    
    private native void printNative(String s);
    private native void printlnNative(String s);
}
