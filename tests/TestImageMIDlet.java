import javax.microedition.midlet.*;
import javax.microedition.lcdui.*;
import java.io.*;

public class TestImageMIDlet extends MIDlet {
    private Display display;
    private ImageCanvas canvas;

    public void startApp() {
        System.out.println("TestImageMIDlet startApp");
        if (display == null) {
            display = Display.getDisplay(this);
            canvas = new ImageCanvas();
            display.setCurrent(canvas);
        }
    }

    public void pauseApp() {}

    public void destroyApp(boolean unconditional) {}

    class ImageCanvas extends Canvas {
        private Image image;

        public ImageCanvas() {
            try {
                System.out.println("Loading /title.gif");
                image = Image.createImage("/title.gif");
                System.out.println("Image loaded: " + image);
            } catch (IOException e) {
                System.out.println("Failed to load image: " + e);
                e.printStackTrace();
            }
        }

        protected void paint(Graphics g) {
            //System.out.println("Canvas paint called");
            g.setColor(0xFFFFFF);
            g.fillRect(0, 0, getWidth(), getHeight());
            
            if (image != null) {
                g.drawImage(image, 0, 0, Graphics.TOP | Graphics.LEFT);
            } else {
                g.setColor(0xFF0000);
                g.drawString("Failed", 0, 0, Graphics.TOP | Graphics.LEFT);
            }
        }
    }
}
