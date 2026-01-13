package javax.microedition.lcdui;

public abstract class Displayable {
    private String title;
    // Commands listener etc...
    
    public String getTitle() { return title; }
    public void setTitle(String s) { title = s; }
    
    public int getWidth() { return 240; }
    public int getHeight() { return 320; }
}
