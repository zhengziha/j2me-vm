package javax.microedition.lcdui;

import java.util.Vector;

public abstract class Displayable {
    private String title;
    private Ticker ticker;
    private CommandListener listener;
    private Vector commands = new Vector();

    public String getTitle() {
        return title;
    }

    public void setTitle(String s) {
        this.title = s;
    }

    public Ticker getTicker() {
        return ticker;
    }

    public void setTicker(Ticker ticker) {
        this.ticker = ticker;
    }

    public boolean isShown() {
        return Display.getDisplay(null).getCurrent() == this;
    }

    public void addCommand(Command cmd) {
        if (cmd != null && !commands.contains(cmd)) {
            commands.addElement(cmd);
        }
    }

    public void removeCommand(Command cmd) {
        commands.removeElement(cmd);
    }

    public void setCommandListener(CommandListener l) {
        this.listener = l;
    }
    
    // Simplified width/height
    public int getWidth() {
        return 240;
    }

    public int getHeight() {
        return 320;
    }

    protected void sizeChanged(int w, int h) {
    }
}
