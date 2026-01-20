package javax.microedition.lcdui;

public class Command {
    public static final int SCREEN = 1;
    public static final int BACK = 2;
    public static final int CANCEL = 3;
    public static final int OK = 4;
    public static final int HELP = 5;
    public static final int STOP = 6;
    public static final int EXIT = 7;
    public static final int ITEM = 8;

    private String label;
    private String longLabel;
    private int type;
    private int priority;

    public Command(String label, int type, int priority) {
        this(label, null, type, priority);
    }

    public Command(String label, String longLabel, int type, int priority) {
        this.label = label;
        this.longLabel = longLabel;
        this.type = type;
        this.priority = priority;
    }

    public String getLabel() {
        return label;
    }

    public String getLongLabel() {
        return longLabel;
    }

    public int getCommandType() {
        return type;
    }

    public int getPriority() {
        return priority;
    }
}
