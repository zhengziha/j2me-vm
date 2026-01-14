import javax.microedition.lcdui.*;
import java.io.InputStream;
public class ResourceTest extends Canvas {
    public ResourceTest() {
        try {
            InputStream is = getClass().getResourceAsStream("/test_res.txt");
            if (is != null) {
                System.out.println("Resource loaded!");
            } else {
                System.out.println("Resource failed to load (null)");
            }
        } catch (Exception e) {
             System.out.println("Exception loading resource");
        }
    }
    protected void paint(Graphics g) {}
}
