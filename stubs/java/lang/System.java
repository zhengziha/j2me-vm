package java.lang;

import java.io.PrintStream;

public final class System {
    private System() {}

    public static final PrintStream out;
    public static final PrintStream err;

    static {
        out = new PrintStream(new java.io.OutputStream() {
            public void write(int b) {
                printNative(String.valueOf((char)b));
            }
        });
        err = out; // Simplify for now
    }

    private static native void printNative(String s);

    public static native long currentTimeMillis();

    public static native void gc();

    public static native void arraycopy(Object src, int src_position, Object dst, int dst_position, int length);

    public static String getProperty(String key) {
        if (key == null) {
            throw new NullPointerException("key is null");
        }
        if (key.equals("microedition.platform")) return "J2ME-VM";
        if (key.equals("microedition.encoding")) return "UTF-8";
        if (key.equals("microedition.profiles")) return "MIDP-2.0";
        if (key.equals("microedition.configuration")) return "CLDC-1.1";
        if (key.equals("microedition.locale")) return "en-US";
        
        return null;
    }

    public static void exit(int status) {
        exitNative(status);
    }
    
    private static native void exitNative(int status);
    
    public static int identityHashCode(Object x) {
        if (x == null) return 0;
        return x.hashCode();
    }
}
