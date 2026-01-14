package javax.microedition.lcdui;
import java.io.InputStream;
import java.io.IOException;

public class Image {
    // We store native pointer as int
    public int nativePtr;

    private Image(int ptr) {
        this.nativePtr = ptr;
    }

    public static Image createImage(String name) throws IOException {
        // InputStream is = new Object().getClass().getResourceAsStream(name);
        // if (is == null) throw new IOException("Resource not found: " + name);
        
        // Call native that returns int
        int ptr = createImageNative(name);
        if (ptr == 0) throw new IOException("Failed to load image: " + name);
        
        return new Image(ptr);
    }
    
    private static native int createImageNative(String name);
    
    public native int getWidth();
    public native int getHeight();
}
