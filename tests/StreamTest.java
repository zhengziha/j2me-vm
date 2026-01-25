import java.io.*;
import javax.microedition.midlet.*;

public class StreamTest extends MIDlet {
    protected void startApp() throws MIDletStateChangeException {
        try {
            System.out.println("Test 1: Reading resource twice");
            readResource("/res.txt");
            readResource("/res.txt");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void readResource(String path) throws IOException {
        InputStream is = getClass().getResourceAsStream(path);
        if (is == null) {
            System.out.println("Resource not found: " + path);
            return;
        }
        System.out.println("Opened stream: " + is);
        System.out.println("Mark supported: " + is.markSupported());
        
        int b1 = is.read();
        System.out.println("Read 1: " + b1);
        
        if (is.markSupported()) {
            is.mark(10);
            System.out.println("Marked.");
            int b2 = is.read();
            System.out.println("Read 2: " + b2);
            is.reset();
            System.out.println("Reset.");
            int b2_again = is.read();
            System.out.println("Read 2 (again): " + b2_again);
            
            if (b2 != b2_again) {
                System.out.println("FAIL: Mark/Reset failed.");
            } else {
                System.out.println("PASS: Mark/Reset working.");
            }
        }
        
        is.close();
    }

    protected void pauseApp() {}
    protected void destroyApp(boolean unconditional) {}
}
