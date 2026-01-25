package com.nokia.mid.ui;

import javax.microedition.lcdui.Font;
import javax.microedition.lcdui.Graphics;
import javax.microedition.lcdui.Image;

public final class DirectUtils {
    private DirectUtils() {
    }

    public static Image createImage(byte[] imageData, int imageOffset, int imageLength) {
        return Image.createImage(imageData, imageOffset, imageLength);
    }

    public static Image createImage(int width, int height, int argb) {
        Image img = Image.createImage(width, height);
        Graphics g = img.getGraphics();
        g.setColor(argb);
        g.fillRect(0, 0, width, height);
        return img;
    }

    public static DirectGraphics getDirectGraphics(Graphics g) {
        return new DirectGraphicsImpl(g);
    }

    public static Font getFont(int face, int style, int height) {
        return Font.getDefaultFont();
    }

    public static Font getFont(int s) {
        return Font.getDefaultFont();
    }
}
