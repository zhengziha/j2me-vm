package java.lang;

import java.io.PrintStream;

public class System {
    public static PrintStream out = new PrintStream();
    
    public static long currentTimeMillis() {
        return currentTimeMillisNative();
    }
    
    public static void arraycopy(Object src, int srcPos, Object dest, int destPos, int length) {
        if (src == null || dest == null) {
            throw new NullPointerException();
        }
        if (srcPos < 0 || destPos < 0 || length < 0 ||
            srcPos + length > ((Object[])src).length ||
            destPos + length > ((Object[])dest).length) {
            throw new ArrayIndexOutOfBoundsException();
        }
        Object[] srcArray = (Object[])src;
        Object[] destArray = (Object[])dest;
        for (int i = 0; i < length; i++) {
            destArray[destPos + i] = srcArray[srcPos + i];
        }
    }
    
    private static native long currentTimeMillisNative();
}
