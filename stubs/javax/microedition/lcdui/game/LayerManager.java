package javax.microedition.lcdui.game;

import javax.microedition.lcdui.Graphics;

public class LayerManager {
    private Layer[] layers;
    private int count;
    private int x, y;
    private int width, height;

    public LayerManager() {
        layers = new Layer[4];
        count = 0;
    }

    public void append(Layer l) {
        if (l == null) throw new NullPointerException();
        ensureCapacity();
        layers[count++] = l;
    }

    public void insert(Layer l, int index) {
        if (l == null) throw new NullPointerException();
        if (index < 0 || index > count) throw new IndexOutOfBoundsException();
        ensureCapacity();
        System.arraycopy(layers, index, layers, index + 1, count - index);
        layers[index] = l;
        count++;
    }

    public Layer getLayerAt(int index) {
        if (index < 0 || index >= count) throw new IndexOutOfBoundsException();
        return layers[index];
    }

    public int getSize() {
        return count;
    }

    public void remove(Layer l) {
        for (int i = 0; i < count; i++) {
            if (layers[i] == l) {
                System.arraycopy(layers, i + 1, layers, i, count - i - 1);
                layers[--count] = null;
                return;
            }
        }
    }

    public void paint(Graphics g, int x, int y) {
        int oldTx = g.getTranslateX();
        int oldTy = g.getTranslateY();
        int clipX = g.getClipX();
        int clipY = g.getClipY();
        int clipW = g.getClipWidth();
        int clipH = g.getClipHeight();
        
        g.translate(x - this.x, y - this.y);
        g.setClip(this.x, this.y, width, height);

        for (int i = 0; i < count; i++) {
            Layer l = layers[i];
            l.paint(g);
        }
        
        g.translate(oldTx - g.getTranslateX(), oldTy - g.getTranslateY());
        g.setClip(clipX, clipY, clipW, clipH);
    }

    public void setViewWindow(int x, int y, int width, int height) {
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
    }

    private void ensureCapacity() {
        if (count == layers.length) {
            Layer[] newLayers = new Layer[count * 2];
            System.arraycopy(layers, 0, newLayers, 0, count);
            layers = newLayers;
        }
    }
}
