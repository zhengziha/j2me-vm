package javax.microedition.lcdui;

import java.util.Vector;

public class List extends Screen implements Choice {
    public static final Command SELECT_COMMAND = new Command("", Command.SCREEN, 0);
    
    private int type;
    private Vector items = new Vector();
    private Vector images = new Vector();
    private boolean[] selected;

    public List(String title, int listType) {
        this(title, listType, null, null);
    }

    public List(String title, int listType, String[] stringElements, Image[] imageElements) {
        setTitle(title);
        this.type = listType;
        if (stringElements != null) {
            for (int i = 0; i < stringElements.length; i++) {
                items.addElement(stringElements[i]);
                images.addElement(imageElements != null && i < imageElements.length ? imageElements[i] : null);
            }
        }
        selected = new boolean[items.size()];
    }
    
    // Implement Choice interface methods (similar to ChoiceGroup but List extends Screen)
    public int size() { return items.size(); }
    public String getString(int elementNum) { return (String)items.elementAt(elementNum); }
    public Image getImage(int elementNum) { return (Image)images.elementAt(elementNum); }
    
    public int append(String stringPart, Image imagePart) {
        items.addElement(stringPart);
        images.addElement(imagePart);
        boolean[] newSelected = new boolean[items.size()];
        System.arraycopy(selected, 0, newSelected, 0, selected.length);
        selected = newSelected;
        return items.size() - 1;
    }
    
    public void insert(int elementNum, String stringPart, Image imagePart) {
        items.insertElementAt(stringPart, elementNum);
        images.insertElementAt(imagePart, elementNum);
    }
    
    public void delete(int elementNum) {
        items.removeElementAt(elementNum);
        images.removeElementAt(elementNum);
    }
    
    public void deleteAll() {
        items.removeAllElements();
        images.removeAllElements();
        selected = new boolean[0];
    }
    
    public void set(int elementNum, String stringPart, Image imagePart) {
        items.setElementAt(stringPart, elementNum);
        images.setElementAt(imagePart, elementNum);
    }
    
    public boolean isSelected(int elementNum) { return selected[elementNum]; }
    public int getSelectedIndex() {
        for (int i = 0; i < selected.length; i++) {
            if (selected[i]) return i;
        }
        return -1;
    }
    
    public int getSelectedFlags(boolean[] selectedArray_return) {
        int count = 0;
        for (int i = 0; i < selected.length && i < selectedArray_return.length; i++) {
            selectedArray_return[i] = selected[i];
            if (selected[i]) count++;
        }
        return count;
    }
    
    public void setSelectedIndex(int elementNum, boolean selected) {
        if (type == EXCLUSIVE || type == IMPLICIT) {
            for (int i = 0; i < this.selected.length; i++) this.selected[i] = false;
        }
        this.selected[elementNum] = selected;
    }
    
    public void setSelectedFlags(boolean[] selectedArray) {}
    public void setFitPolicy(int fitPolicy) {}
    public int getFitPolicy() { return 0; }
    public void setFont(int elementNum, Font font) {}
    public Font getFont(int elementNum) { return Font.getDefaultFont(); }
    
    public void setSelectCommand(Command command) {}
}
