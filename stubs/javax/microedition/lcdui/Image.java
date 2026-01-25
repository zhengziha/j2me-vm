package javax.microedition.lcdui;

import java.io.InputStream;
import java.io.IOException;

public class Image {
    public int nativePtr;

    private Image(int ptr) {
        this.nativePtr = ptr;
    }

    public static Image createImage(int width, int height) {
        int ptr = createMutableImageNative(width, height);
        return new Image(ptr);
    }

    public static Image createImage(Image source) {
        int ptr = createImmutableCopyNative(source.nativePtr);
        return new Image(ptr);
    }

    public static Image createImage(String name) throws IOException {
        int ptr = createImageNative(name);
        if (ptr == 0) throw new IOException("Failed to load image: " + name);
        return new Image(ptr);
    }

    public static Image createImage(byte[] imageData, int imageOffset, int imageLength) {
        int ptr = createImageFromDataNative(imageData, imageOffset, imageLength);
        if (ptr == 0) throw new IllegalArgumentException("Invalid image data");
        return new Image(ptr);
    }

    public static Image createImage(InputStream stream) throws IOException {
        if (stream == null) throw new NullPointerException("InputStream cannot be null");
        
        java.io.ByteArrayOutputStream buffer = new java.io.ByteArrayOutputStream();
        int nRead;
        byte[] data = new byte[4096];
        while ((nRead = stream.read(data, 0, data.length)) != -1) {
            buffer.write(data, 0, nRead);
        }
        buffer.flush();
        byte[] imageData = buffer.toByteArray();
        
        return createImage(imageData, 0, imageData.length);
    }
    
    public static Image createRGBImage(int[] rgb, int width, int height, boolean processAlpha) {
        int ptr = createRGBImageNative(rgb, width, height, processAlpha);
        return new Image(ptr);
    }

    public Graphics getGraphics() {
        if (!isMutable()) throw new IllegalStateException("Image is immutable");
        int gPtr = getGraphicsNative(nativePtr);
        Graphics g = new Graphics(gPtr);
        g.setClip(0, 0, getWidth(), getHeight());
        return g;
    }

    public boolean isMutable() {
        return isMutableNative(nativePtr);
    }

    public native int getWidth();
    public native int getHeight();
    
    public void getRGB(int[] rgbData, int offset, int scanlength, int x, int y, int width, int height) {
        getRGBNative(nativePtr, rgbData, offset, scanlength, x, y, width, height);
    }

    public int getNativePtr() {
        return nativePtr;
    }

    private static native int createImageNative(String name);
    private static native int createMutableImageNative(int width, int height);
    private static native int createImmutableCopyNative(int srcPtr);
    private static native int createImageFromDataNative(byte[] data, int offset, int length);
    private static native int createRGBImageNative(int[] rgb, int width, int height, boolean processAlpha);
    private native boolean isMutableNative(int ptr);
    private native int getGraphicsNative(int imgPtr);
    private native void getRGBNative(int ptr, int[] rgbData, int offset, int scanlength, int x, int y, int width, int height);
}
