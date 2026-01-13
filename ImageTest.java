import javax.microedition.lcdui.*;
import java.io.IOException;
public class ImageTest extends Canvas {
    Image img;
    public ImageTest() {
        try {
            // Load test image
            img = Image.createImage("/test_image.png");
            System.out.println("Image loaded successfully: " + img);
        } catch (IOException e) {
             System.out.println("Failed to load image: " + e);
        }
    }
    
    protected void paint(Graphics g) {
        g.setColor(255, 255, 255);
        g.fillRect(0, 0, 240, 320);
        
        if (img != null) {
            g.drawImage(img, 50, 50, 0);
        } else {
            g.setColor(255, 0, 0);
            g.drawString("No Image", 10, 10, 0);
        }
    }
}
