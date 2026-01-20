package javax.microedition.lcdui;

public class Graphics {
    public static final int HCENTER = 1;
    public static final int VCENTER = 2;
    public static final int LEFT = 4;
    public static final int RIGHT = 8;
    public static final int TOP = 16;
    public static final int BOTTOM = 32;
    public static final int BASELINE = 64;
    
    public static final int SOLID = 0;
    public static final int DOTTED = 1;

    private int tx, ty;
    private int clipX, clipY, clipWidth, clipHeight;
    private int color;
    private Font font;
    private int nativePtr; // For native context if needed

    public Graphics() {
        // Default constructor
    }

    public Graphics(int ptr) {
        this.nativePtr = ptr;
    }
    // or we can implement the logic in Java where possible (like translation).

    public void translate(int x, int y) {
        this.tx += x;
        this.ty += y;
        translateNative(x, y);
    }

    public int getTranslateX() {
        return tx;
    }

    public int getTranslateY() {
        return ty;
    }

    public int getClipX() {
        return clipX;
    }

    public int getClipY() {
        return clipY;
    }

    public int getClipWidth() {
        return clipWidth;
    }

    public int getClipHeight() {
        return clipHeight;
    }

    public void setClip(int x, int y, int width, int height) {
        this.clipX = x;
        this.clipY = y;
        this.clipWidth = width;
        this.clipHeight = height;
        setClipNative(x, y, width, height);
    }

    public void clipRect(int x, int y, int width, int height) {
        // Intersect with current clip
        // Simplified intersection logic
        int newX = x + tx;
        int newY = y + ty;
        
        int x1 = Math.max(clipX, newX);
        int y1 = Math.max(clipY, newY);
        int x2 = Math.min(clipX + clipWidth, newX + width);
        int y2 = Math.min(clipY + clipHeight, newY + height);
        
        this.clipX = x1;
        this.clipY = y1;
        this.clipWidth = Math.max(0, x2 - x1);
        this.clipHeight = Math.max(0, y2 - y1);
        
        setClipNative(clipX, clipY, clipWidth, clipHeight);
    }

    public void setColor(int red, int green, int blue) {
        this.color = (red << 16) | (green << 8) | blue;
        setColorNative(red, green, blue);
    }

    public void setColor(int RGB) {
        this.color = RGB & 0x00FFFFFF;
        setColorNative((RGB >> 16) & 0xFF, (RGB >> 8) & 0xFF, RGB & 0xFF);
    }

    public int getColor() {
        return color;
    }
    
    public void setFont(Font font) {
        this.font = font;
        setFontNative(font);
    }
    
    public Font getFont() {
        return font;
    }

    public void drawLine(int x1, int y1, int x2, int y2) {
        drawLineNative(x1 + tx, y1 + ty, x2 + tx, y2 + ty);
    }

    public void fillRect(int x, int y, int width, int height) {
        fillRectNative(x + tx, y + ty, width, height);
    }
    
    public void drawRect(int x, int y, int width, int height) {
        drawRectNative(x + tx, y + ty, width, height);
    }
    
    public void drawRoundRect(int x, int y, int width, int height, int arcWidth, int arcHeight) {
        drawRoundRectNative(x + tx, y + ty, width, height, arcWidth, arcHeight);
    }
    
    public void fillRoundRect(int x, int y, int width, int height, int arcWidth, int arcHeight) {
        fillRoundRectNative(x + tx, y + ty, width, height, arcWidth, arcHeight);
    }
    
    public void fillArc(int x, int y, int width, int height, int startAngle, int arcAngle) {
        fillArcNative(x + tx, y + ty, width, height, startAngle, arcAngle);
    }
    
    public void drawArc(int x, int y, int width, int height, int startAngle, int arcAngle) {
        drawArcNative(x + tx, y + ty, width, height, startAngle, arcAngle);
    }

    public void drawString(String str, int x, int y, int anchor) {
        drawStringNative(str, x + tx, y + ty, anchor);
    }
    
    public void drawSubstring(String str, int offset, int len, int x, int y, int anchor) {
        drawString(str.substring(offset, offset + len), x, y, anchor);
    }
    
    public void drawChar(char character, int x, int y, int anchor) {
        drawString(String.valueOf(character), x, y, anchor);
    }
    
    public void drawChars(char[] data, int offset, int length, int x, int y, int anchor) {
        drawString(new String(data, offset, length), x, y, anchor);
    }

    public void drawImage(Image img, int x, int y, int anchor) {
        drawImageNative(img, x + tx, y + ty, anchor);
    }

    public void drawRegion(Image src, int x_src, int y_src, int width, int height, int transform, int x_dest, int y_dest, int anchor) {
        drawRegionNative(src, x_src, y_src, width, height, transform, x_dest + tx, y_dest + ty, anchor);
    }

    public void copyArea(int x_src, int y_src, int width, int height, int x_dest, int y_dest, int anchor) {
        copyAreaNative(x_src + tx, y_src + ty, width, height, x_dest + tx, y_dest + ty, anchor);
    }

    public void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3) {
        fillTriangleNative(x1 + tx, y1 + ty, x2 + tx, y2 + ty, x3 + tx, y3 + ty);
    }
    
    public void drawRGB(int[] rgbData, int offset, int scanlength, int x, int y, int width, int height, boolean processAlpha) {
        drawRGBNative(rgbData, offset, scanlength, x + tx, y + ty, width, height, processAlpha);
    }

    private native void drawLineNative(int x1, int y1, int x2, int y2);
    private native void fillRectNative(int x, int y, int width, int height);
    private native void drawRectNative(int x, int y, int width, int height);
    private native void drawRoundRectNative(int x, int y, int width, int height, int arcWidth, int arcHeight);
    private native void fillRoundRectNative(int x, int y, int width, int height, int arcWidth, int arcHeight);
    private native void fillArcNative(int x, int y, int width, int height, int startAngle, int arcAngle);
    private native void drawArcNative(int x, int y, int width, int height, int startAngle, int arcAngle);
    private native void setColorNative(int red, int green, int blue);
    private native void drawStringNative(String str, int x, int y, int anchor);
    private native void drawImageNative(Image img, int x, int y, int anchor);
    private native void drawRegionNative(Image src, int x_src, int y_src, int width, int height, int transform, int x_dest, int y_dest, int anchor);
    private native void copyAreaNative(int x_src, int y_src, int width, int height, int x_dest, int y_dest, int anchor);
    private native void fillTriangleNative(int x1, int y1, int x2, int y2, int x3, int y3);
    private native void drawRGBNative(int[] rgbData, int offset, int scanlength, int x, int y, int width, int height, boolean processAlpha);
    private native void translateNative(int x, int y);
    private native void setClipNative(int x, int y, int width, int height);
    private native void setFontNative(Font font);
}
