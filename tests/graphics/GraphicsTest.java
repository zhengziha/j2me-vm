import javax.microedition.lcdui.*;
public class GraphicsTest extends Canvas {
    public GraphicsTest() { }
    protected void paint(Graphics g) {
        g.setColor(255, 0, 0); // Red
        g.fillRect(10, 10, 100, 50);
        g.setColor(0, 255, 0); // Green
        g.drawLine(0, 0, 240, 320);
    }
}
