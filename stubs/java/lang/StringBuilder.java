package java.lang;

public class StringBuilder {
    public native void init(); // Helper for native init if needed, or just make constructors native

    // We can't make constructors native in Java syntax directly easily without caveats, 
    // but in J2ME/stubs it's allowed if the VM supports it.
    // However, usually constructors call super().
    
    // Better: call a native init method inside the constructor.
    
    public StringBuilder() {
        initNative();
    }

    public StringBuilder(String str) {
        initNative();
        append(str);
    }
    
    private native void initNative();

    public native StringBuilder append(String str);

    public native StringBuilder append(int i);

    public native StringBuilder append(long l);

    public native StringBuilder append(boolean b);

    public native StringBuilder append(float f);

    public native StringBuilder append(double d);

    public native StringBuilder append(char c);

    public native StringBuilder append(Object obj);

    public native String toString();
}
