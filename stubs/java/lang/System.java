package java.lang;

import java.io.PrintStream;

public class System {
    public static PrintStream out = new PrintStream();
    
    public static long currentTimeMillis() {
        return currentTimeMillisNative();
    }
    
    private static native long currentTimeMillisNative();
}
