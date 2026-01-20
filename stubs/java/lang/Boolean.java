package java.lang;

public class Boolean {
    public static final Boolean TRUE = new Boolean(true);
    public static final Boolean FALSE = new Boolean(false);

    private boolean value;

    public Boolean(boolean value) {
        this.value = value;
    }

    public boolean booleanValue() {
        return value;
    }

    public String toString() {
        return value ? "true" : "false";
    }

    public int hashCode() {
        return value ? 1231 : 1237;
    }

    public boolean equals(Object obj) {
        if (obj instanceof Boolean) {
            return value == ((Boolean)obj).booleanValue();
        }
        return false;
    }
}
