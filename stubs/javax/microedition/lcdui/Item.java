package javax.microedition.lcdui;

public abstract class Item {
    public static final int PLAIN = 0;
    public static final int HYPERLINK = 1;
    public static final int BUTTON = 2;

    public static final int LAYOUT_DEFAULT = 0;
    public static final int LAYOUT_LEFT = 1;
    public static final int LAYOUT_RIGHT = 2;
    public static final int LAYOUT_CENTER = 3;
    public static final int LAYOUT_TOP = 16;
    public static final int LAYOUT_BOTTOM = 32;
    public static final int LAYOUT_VCENTER = 48;
    public static final int LAYOUT_NEWLINE_BEFORE = 256;
    public static final int LAYOUT_NEWLINE_AFTER = 512;
    public static final int LAYOUT_SHRINK = 1024;
    public static final int LAYOUT_EXPAND = 2048;
    public static final int LAYOUT_VSHRINK = 4096;
    public static final int LAYOUT_VEXPAND = 8192;
    public static final int LAYOUT_2 = 16384;

    private String label;
    private int layout;

    public void setLabel(String label) {
        this.label = label;
    }

    public String getLabel() {
        return label;
    }

    public int getLayout() {
        return layout;
    }

    public void setLayout(int layout) {
        this.layout = layout;
    }
    
    public void setPreferredSize(int width, int height) {}
    public int getPreferredWidth() { return 0; }
    public int getPreferredHeight() { return 0; }
    public void setDefaultCommand(Command cmd) {}
    public void setItemCommandListener(ItemCommandListener l) {}
    public void notifyStateChanged() {}
}
