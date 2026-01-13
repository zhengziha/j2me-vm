package java.lang;
import java.io.InputStream;
public class Class {
    private String name;
    public native InputStream getResourceAsStream(String name);
    public String getName() { return name; }
}
