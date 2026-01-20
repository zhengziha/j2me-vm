package javax.microedition.lcdui.game;

import javax.microedition.lcdui.Canvas;
import javax.microedition.lcdui.Graphics;
import javax.microedition.lcdui.Image;

public abstract class GameCanvas extends Canvas {
    public static final int UP_PRESSED = 1 << Canvas.UP;
    public static final int DOWN_PRESSED = 1 << Canvas.DOWN;
    public static final int LEFT_PRESSED = 1 << Canvas.LEFT;
    public static final int RIGHT_PRESSED = 1 << Canvas.RIGHT;
    public static final int FIRE_PRESSED = 1 << Canvas.FIRE;
    public static final int GAME_A_PRESSED = 1 << Canvas.GAME_A;
    public static final int GAME_B_PRESSED = 1 << Canvas.GAME_B;
    public static final int GAME_C_PRESSED = 1 << Canvas.GAME_C;
    public static final int GAME_D_PRESSED = 1 << Canvas.GAME_D;

    private Image offscreen;
    private Graphics graphics;
    private int keyStates;

    protected GameCanvas(boolean suppressKeyEvents) {
        // Initialize offscreen buffer
        // In a real implementation, we need to know the screen size.
        // Canvas usually has width/height.
        // But constructor runs before we might know size?
        // Usually we fetch size from Display or similar.
        // For stubs, we can delay initialization or assume a default size.
        // Let's assume 240x320 for now or fetch from native.
        
        int w = getWidth();
        int h = getHeight();
        if (w <= 0) w = 240;
        if (h <= 0) h = 320;
        
        offscreen = Image.createImage(w, h);
        graphics = offscreen.getGraphics();
    }

    protected Graphics getGraphics() {
        return graphics;
    }

    public int getKeyStates() {
        int states = getKeyStatesNative();
        // Native method should return bitmask of pressed keys
        return states;
    }

    public void flushGraphics(int x, int y, int width, int height) {
        flushGraphicsNative(offscreen, x, y, width, height);
    }

    public void flushGraphics() {
        flushGraphics(0, 0, offscreen.getWidth(), offscreen.getHeight());
    }

    public void paint(Graphics g) {
        // Default implementation just draws the offscreen buffer
        g.drawImage(offscreen, 0, 0, Graphics.TOP | Graphics.LEFT);
    }
    
    // Native binding for key states polling
    private native int getKeyStatesNative();
    
    // Native binding to flush offscreen to screen
    // Actually, flushGraphics usually forces a repaint or blits directly.
    // In MIDP, Canvas.repaint() schedules a paint().
    // flushGraphics() is immediate.
    private native void flushGraphicsNative(Image offscreen, int x, int y, int width, int height);
}
