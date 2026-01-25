package com.nokia.mid.ui;

import javax.microedition.lcdui.Graphics;
import javax.microedition.lcdui.Image;

final class DirectGraphicsImpl implements DirectGraphics {
    private final Graphics g;
    private int argb;

    DirectGraphicsImpl(Graphics g) {
        this.g = g;
    }

    public void setARGBColor(int argb) {
        this.argb = argb;
        g.setColor(argb);
    }

    public int getAlphaComponent() {
        return (argb >>> 24) & 0xFF;
    }

    public int getNativePixelFormat() {
        return TYPE_INT_8888_ARGB;
    }

    public void drawImage(Image image, int x, int y, int anchor, int transform) {
        g.drawImage(image, x, y, anchor);
    }

    public void drawPixels(int[] pixels, boolean processAlpha, int offset, int scanlength, int x, int y, int width, int height, int manipulation, int format) {
        g.drawRGB(pixels, offset, scanlength, x, y, width, height, processAlpha);
    }

    public void drawPixels(byte[] pixels, byte[] mask, int offset, int scanlength, int x, int y, int width, int height, int manipulation, int format) {
        int[] rgb = new int[width * height];
        int in = offset;
        int out = 0;
        for (int row = 0; row < height; row++) {
            int idx = in;
            for (int col = 0; col < width; col++) {
                int v = pixels[idx] & 0xFF;
                rgb[out++] = (0xFF << 24) | (v << 16) | (v << 8) | v;
                idx++;
            }
            in += scanlength;
        }
        g.drawRGB(rgb, 0, width, x, y, width, height, true);
    }

    public void drawPixels(short[] pixels, boolean processAlpha, int offset, int scanlength, int x, int y, int width, int height, int manipulation, int format) {
        int[] rgb = new int[width * height];
        int in = offset;
        int out = 0;
        for (int row = 0; row < height; row++) {
            int idx = in;
            for (int col = 0; col < width; col++) {
                int v = pixels[idx] & 0xFFFF;
                int r = ((v >> 11) & 0x1F) << 3;
                int g2 = ((v >> 5) & 0x3F) << 2;
                int b = (v & 0x1F) << 3;
                rgb[out++] = (0xFF << 24) | (r << 16) | (g2 << 8) | b;
                idx++;
            }
            in += scanlength;
        }
        g.drawRGB(rgb, 0, width, x, y, width, height, true);
    }

    public void getPixels(int[] pixels, int offset, int scanlength, int x, int y, int width, int height, int format) {
    }

    public void getPixels(byte[] pixels, byte[] mask, int offset, int scanlength, int x, int y, int width, int height, int format) {
    }

    public void getPixels(short[] pixels, int offset, int scanlength, int x, int y, int width, int height, int format) {
    }

    public void drawPolygon(int[] xPoints, int xOffset, int[] yPoints, int yOffset, int nPoints, int argbColor) {
    }

    public void fillPolygon(int[] xPoints, int xOffset, int[] yPoints, int yOffset, int nPoints, int argbColor) {
    }

    public void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int argbColor) {
    }

    public void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int argbColor) {
        g.setColor(argbColor);
        g.fillTriangle(x1, y1, x2, y2, x3, y3);
    }
}

