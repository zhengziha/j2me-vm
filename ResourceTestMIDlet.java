import javax.microedition.midlet.*;
import javax.microedition.lcdui.*;
import java.io.*;

public class ResourceTestMIDlet extends MIDlet implements CommandListener {
    private Display display;
    private Form form;
    private Command exitCommand;
    
    public ResourceTestMIDlet() {
        display = Display.getDisplay(this);
        form = new Form("Resource Test");
        exitCommand = new Command("Exit", Command.EXIT, 1);
        form.addCommand(exitCommand);
        form.setCommandListener(this);
    }
    
    public void startApp() {
        testResourceAsStream();
        display.setCurrent(form);
    }
    
    public void pauseApp() {
    }
    
    public void destroyApp(boolean unconditional) {
    }
    
    public void commandAction(Command c, Displayable d) {
        if (c == exitCommand) {
            destroyApp(false);
            notifyDestroyed();
        }
    }
    
    private void testResourceAsStream() {
        form.append("Testing Class.getResourceAsStream()...\n");
        
        try {
            // Test with a resource that should exist
            InputStream is = getClass().getResourceAsStream("/META-INF/MANIFEST.MF");
            
            form.append("InputStream obtained: " + is + "\n");
            form.append("Is ByteArrayInputStream: " + (is instanceof ByteArrayInputStream) + "\n");
            
            if (is != null) {
                // Read and print the resource content
                form.append("\nResource content:\n");
                int b;
                StringBuffer content = new StringBuffer();
                while ((b = is.read()) != -1) {
                    content.append((char)b);
                }
                form.append(content.toString() + "\n");
                is.close();
            } else {
                form.append("Resource not found!\n");
            }
            
        } catch (Exception e) {
            form.append("Error: " + e.getMessage() + "\n");
            e.printStackTrace();
        }
        
        form.append("\nTest completed.\n");
    }
}
