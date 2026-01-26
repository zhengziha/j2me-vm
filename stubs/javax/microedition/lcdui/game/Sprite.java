package javax.microedition.lcdui.game;

import javax.microedition.lcdui.Graphics;
import javax.microedition.lcdui.Image;

public class Sprite extends Layer {
    public static final int TRANS_MIRROR = 2;
    public static final int TRANS_MIRROR_ROT180 = 1;
    public static final int TRANS_MIRROR_ROT270 = 4;
    public static final int TRANS_MIRROR_ROT90 = 7;
    public static final int TRANS_NONE = 0;
    public static final int TRANS_ROT180 = 3;
    public static final int TRANS_ROT270 = 6;
    public static final int TRANS_ROT90 = 5;

    private Image sourceImage;
    private int numberFrames;
    private int[] frameSequence;
    private int frame;
    private int frameWidth;
    private int frameHeight;
    private int refX;
    private int refY;
    private int transform = TRANS_NONE;

    public Sprite(Image image) {
        super(0, 0, image.getWidth(), image.getHeight(), true);
        initialize(image, image.getWidth(), image.getHeight());
    }

    public Sprite(Image image, int frameWidth, int frameHeight) {
        super(0, 0, frameWidth, frameHeight, true);
        initialize(image, frameWidth, frameHeight);
    }

    public Sprite(Sprite s) {
        super(s.getX(), s.getY(), s.getWidth(), s.getHeight(), s.isVisible());
        this.sourceImage = s.sourceImage;
        this.frameWidth = s.frameWidth;
        this.frameHeight = s.frameHeight;
        this.numberFrames = s.numberFrames;
        this.frameSequence = s.frameSequence; // Share sequence or copy? Spec says share if not changed? No, it says "same frame sequence"
        this.frame = 0;
        this.refX = s.refX;
        this.refY = s.refY;
        this.transform = s.transform;
    }

    private void initialize(Image image, int fWidth, int fHeight) {
        this.sourceImage = image;
        this.frameWidth = fWidth;
        this.frameHeight = fHeight;
        int imgWidth = image.getWidth();
        int imgHeight = image.getHeight();
        if (imgWidth % fWidth != 0 || imgHeight % fHeight != 0) {
            throw new IllegalArgumentException("Image size is not a multiple of frame size");
        }
        int cols = imgWidth / fWidth;
        int rows = imgHeight / fHeight;
        this.numberFrames = cols * rows;
        this.frameSequence = new int[numberFrames];
        for (int i = 0; i < numberFrames; i++) {
            frameSequence[i] = i;
        }
        this.frame = 0;
    }

    public void defineReferencePixel(int x, int y) {
        this.refX = x;
        this.refY = y;
    }

    public void setRefPixelPosition(int x, int y) {
        setPosition(x - refX, y - refY);
    }

    public int getRefPixelX() {
        return getX() + refX;
    }

    public int getRefPixelY() {
        return getY() + refY;
    }

    public void setFrame(int sequenceIndex) {
        if (sequenceIndex < 0 || sequenceIndex >= frameSequence.length) {
            throw new IndexOutOfBoundsException();
        }
        this.frame = sequenceIndex;
    }

    public int getFrame() {
        return frame;
    }

    public int getRawFrameCount() {
        return numberFrames;
    }

    public int getFrameSequenceLength() {
        return frameSequence.length;
    }

    public void nextFrame() {
        frame = (frame + 1) % frameSequence.length;
    }

    public void prevFrame() {
        frame = (frame - 1 + frameSequence.length) % frameSequence.length;
    }

    public void setFrameSequence(int[] sequence) {
        if (sequence == null) {
            // Restore default
            this.frameSequence = new int[numberFrames];
            for (int i = 0; i < numberFrames; i++) {
                frameSequence[i] = i;
            }
        } else {
            for (int i = 0; i < sequence.length; i++) {
                if (sequence[i] < 0 || sequence[i] >= numberFrames) {
                    throw new ArrayIndexOutOfBoundsException();
                }
            }
            this.frameSequence = sequence;
        }
        this.frame = 0;
    }

    public void setImage(Image img, int frameWidth, int frameHeight) {
        initialize(img, frameWidth, frameHeight);
        setWidth(frameWidth);
        setHeight(frameHeight);
    }
    
    // Internal helper to set width/height on Layer, since fields are package private
    private void setWidth(int w) {
        this.width = w;
    }
    private void setHeight(int h) {
        this.height = h;
    }

    public void setTransform(int transform) {
        this.transform = transform;
    }

    public final void paint(Graphics g) {
        if (!isVisible()) return;
        
        int currentFrameIndex = frameSequence[frame];
        int cols = sourceImage.getWidth() / frameWidth;
        int row = currentFrameIndex / cols;
        int col = currentFrameIndex % cols;
        
        int srcX = col * frameWidth;
        int srcY = row * frameHeight;
        
        // Handling transform is complex using standard Graphics.drawRegion
        // assuming Graphics has drawRegion (MIDP 2.0)
        
        g.drawRegion(sourceImage, srcX, srcY, frameWidth, frameHeight, transform, getX(), getY(), Graphics.TOP | Graphics.LEFT);
    }

    public final boolean collidesWith(Sprite s, boolean pixelLevel) {
        // Simplified AABB collision
        if (!this.isVisible() || !s.isVisible()) return false;
        
        int x1 = this.getX();
        int y1 = this.getY();
        int w1 = this.getWidth();
        int h1 = this.getHeight();
        
        int x2 = s.getX();
        int y2 = s.getY();
        int w2 = s.getWidth();
        int h2 = s.getHeight();
        
        boolean aabb = (x1 < x2 + w2) && (x1 + w1 > x2) && (y1 < y2 + h2) && (y1 + h1 > y2);
        
        if (!aabb) return false;
        if (!pixelLevel) return true;
        
        // Pixel level collision
        return collidesWithNative(s);
    }
    
    public final boolean collidesWith(TiledLayer t, boolean pixelLevel) {
        // Simplified AABB collision
        if (!this.isVisible() || !t.isVisible()) return false;
        
        int x1 = this.getX();
        int y1 = this.getY();
        int w1 = this.getWidth();
        int h1 = this.getHeight();
        
        int x2 = t.getX();
        int y2 = t.getY();
        int w2 = t.getWidth();
        int h2 = t.getHeight();
        
        return (x1 < x2 + w2) && (x1 + w1 > x2) && (y1 < y2 + h2) && (y1 + h1 > y2);
    }
    
    public final boolean collidesWith(Image image, int x, int y, boolean pixelLevel) {
        // Simplified AABB collision with image at position
        if (!this.isVisible()) return false;
        
        int x1 = this.getX();
        int y1 = this.getY();
        int w1 = this.getWidth();
        int h1 = this.getHeight();
        
        int imgW = image.getWidth();
        int imgH = image.getHeight();
        
        return (x1 < x + imgW) && (x1 + w1 > x) && (y1 < y + imgH) && (y1 + h1 > y);
    }

    private native boolean collidesWithNative(Sprite s);
}
