package javax.microedition.lcdui;

public class ImageItem extends Item {
    public static final int LAYOUT_DEFAULT = 0;
    public static final int LAYOUT_LEFT = 1;
    public static final int LAYOUT_RIGHT = 2;
    public static final int LAYOUT_CENTER = 3;
    public static final int LAYOUT_NEWLINE_BEFORE = 256;
    public static final int LAYOUT_NEWLINE_AFTER = 512;

    private Image img;
    private String altText;

    public ImageItem(String label, Image img, int layout, String altText) {
        setLabel(label);
        setImage(img);
        setLayout(layout);
        this.altText = altText;
    }
    
    public ImageItem(String label, Image img, int layout, String altText, int appearanceMode) {
        this(label, img, layout, altText);
    }

    public Image getImage() {
        return img;
    }

    public void setImage(Image img) {
        this.img = img;
    }

    public String getAltText() {
        return altText;
    }

    public void setAltText(String text) {
        this.altText = text;
    }
}
