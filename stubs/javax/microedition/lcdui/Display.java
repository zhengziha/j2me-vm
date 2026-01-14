package javax.microedition.lcdui;
import javax.microedition.midlet.MIDlet;

public class Display {
    private Displayable current;

    private Display() {}

    public static Display getDisplay(MIDlet m) {
        return new Display();
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
