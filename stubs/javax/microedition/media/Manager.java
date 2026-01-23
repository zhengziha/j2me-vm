package javax.microedition.media;

import java.io.InputStream;
import java.io.IOException;

public class Manager {
    public static final String TONE_DEVICE_LOCATOR = "device/tone";

    public static native Player createPlayer(String locator) throws IOException, MediaException;

    public static native Player createPlayer(InputStream stream, String type) throws IOException, MediaException;
}
