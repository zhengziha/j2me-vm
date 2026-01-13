package javax.microedition.lcdui;
import javax.microedition.midlet.MIDlet;

public class Display {
    private static Display instance;
    private Displayable current;

    private Display() {}

    public static Display getDisplay(MIDlet m) {
        if (instance == null) instance = new Display();
        return instance;
    }

    public void setCurrent(Displayable nextDisplayable) {
        current = nextDisplayable;
        setCurrentNative(nextDisplayable);
    }
    
    public Displayable getCurrent() {
        return current;
    }
    
    private native void setCurrentNative(Displayable d);
}
