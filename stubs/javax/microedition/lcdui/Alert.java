package javax.microedition.lcdui;

public class Alert extends Screen {
    public static final int FOREVER = -2;
    private String string;
    private Image image;
    private AlertType type;
    private int timeout;
    private Gauge indicator;

    public Alert(String title) {
        this(title, null, null, null);
    }

    public Alert(String title, String alertText, Image alertImage, AlertType alertType) {
        setTitle(title);
        this.string = alertText;
        this.image = alertImage;
        this.type = alertType;
        this.timeout = FOREVER;
    }

    public int getDefaultTimeout() {
        return FOREVER;
    }

    public int getTimeout() {
        return timeout;
    }

    public void setTimeout(int time) {
        this.timeout = time;
    }

    public AlertType getType() {
        return type;
    }

    public void setType(AlertType type) {
        this.type = type;
    }

    public String getString() {
        return string;
    }

    public void setString(String str) {
        this.string = str;
    }

    public Image getImage() {
        return image;
    }

    public void setImage(Image img) {
        this.image = img;
    }

    public void setIndicator(Gauge indicator) {
        this.indicator = indicator;
    }

    public Gauge getIndicator() {
        return indicator;
    }
}
