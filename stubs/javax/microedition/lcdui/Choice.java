package javax.microedition.lcdui;

public interface Choice {
    public static final int EXCLUSIVE = 1;
    public static final int MULTIPLE = 2;
    public static final int IMPLICIT = 3;
    public static final int POPUP = 4;
    public static final int TEXT_WRAP_DEFAULT = 0;
    public static final int TEXT_WRAP_ON = 1;
    public static final int TEXT_WRAP_OFF = 2;

    int size();
    String getString(int elementNum);
    Image getImage(int elementNum);
    int append(String stringPart, Image imagePart);
    void insert(int elementNum, String stringPart, Image imagePart);
    void delete(int elementNum);
    void deleteAll();
    void set(int elementNum, String stringPart, Image imagePart);
    boolean isSelected(int elementNum);
    int getSelectedIndex();
    int getSelectedFlags(boolean[] selectedArray_return);
    void setSelectedIndex(int elementNum, boolean selected);
    void setSelectedFlags(boolean[] selectedArray);
    void setFitPolicy(int fitPolicy);
    int getFitPolicy();
    void setFont(int elementNum, Font font);
    Font getFont(int elementNum);
}
