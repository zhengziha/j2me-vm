import javax.microedition.midlet.*;
import javax.microedition.lcdui.*;

public class RMSTestMIDlet extends MIDlet {
    private Display display;
    private RMSTest canvas;

    public RMSTestMIDlet() {
        display = Display.getDisplay(this);
        canvas = new RMSTest();
    }

    protected void startApp() {
        display.setCurrent(canvas);
    }

    protected void pauseApp() {
    }

    protected void destroyApp(boolean unconditional) {
    }
}
