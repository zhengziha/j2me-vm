package javax.microedition.lcdui;

public class Form extends Screen {
    private java.util.Vector items = new java.util.Vector();

    public Form(String title) {
        setTitle(title);
    }

    public Form(String title, Item[] items) {
        setTitle(title);
        if (items != null) {
            for (int i = 0; i < items.length; i++) {
                this.items.addElement(items[i]);
            }
        }
    }

    public int append(Item item) {
        items.addElement(item);
        return items.size() - 1;
    }

    public int append(String str) {
        return append(new StringItem(null, str));
    }

    public int append(Image img) {
        return append(new ImageItem(null, img, ImageItem.LAYOUT_DEFAULT, null));
    }

    public void insert(int index, Item item) {
        items.insertElementAt(item, index);
    }

    public void delete(int index) {
        items.removeElementAt(index);
    }

    public void deleteAll() {
        items.removeAllElements();
    }

    public void set(int index, Item item) {
        items.setElementAt(item, index);
    }

    public Item get(int index) {
        return (Item)items.elementAt(index);
    }

    public int size() {
        return items.size();
    }
    
    public void setItemStateListener(ItemStateListener iListener) {}
}
