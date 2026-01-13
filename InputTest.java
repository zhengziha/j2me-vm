import javax.microedition.lcdui.*;
public class InputTest extends Canvas {
    int x = 50;
    int y = 50;
    String lastKey = "None";

    public InputTest() { }

    protected void paint(Graphics g) {
        g.setColor(255, 255, 255);
        g.fillRect(0, 0, 240, 320); // Clear screen
        
        g.setColor(0, 0, 0);
        g.drawString("Press Keys!", 10, 20, 0);
        
        g.setColor(255, 0, 0);
        g.fillRect(x, y, 20, 20); // Player
        
        g.setColor(0, 0, 255);
        g.drawString("Key: " + lastKey, 10, 100, 0);
    }
    
    protected void keyPressed(int keyCode) {
        if (keyCode == UP) y -= 5;
        else if (keyCode == DOWN) y += 5;
        else if (keyCode == LEFT) x -= 5;
        else if (keyCode == RIGHT) x += 5;
        
        lastKey = "" + keyCode;
        repaint(); // In our VM repaint is ignored, but main loop repaints anyway via native update
        
        // Hack: print to console
        System.out.println("Key Pressed: " + keyCode);
    }
}
