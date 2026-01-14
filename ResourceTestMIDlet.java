import javax.microedition.midlet.*;
import javax.microedition.lcdui.*;

public class ResourceTestMIDlet extends MIDlet {
    private Display display;
    private ResourceTest canvas;

    public ResourceTestMIDlet() {
        display = Display.getDisplay(this);
        canvas = new ResourceTest();
    }

    protected void startApp() {
        display.setCurrent(canvas);
    }

    protected void pauseApp() {
    }

    protected void destroyApp(boolean unconditional) {
    }
}
