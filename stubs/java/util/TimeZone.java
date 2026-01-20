package java.util;

public abstract class TimeZone {
    public static TimeZone getDefault() {
        return new SimpleTimeZone(0, "GMT"); // Default to GMT for now
    }
    
    public static TimeZone getTimeZone(String ID) {
        return new SimpleTimeZone(0, ID);
    }
    
    public abstract int getRawOffset();
    public abstract boolean useDaylightTime();
    public abstract String getID();
    
    // In J2ME, TimeZone is abstract but usually has a concrete implementation or factory
}

class SimpleTimeZone extends TimeZone {
    private int rawOffset;
    private String ID;
    
    public SimpleTimeZone(int rawOffset, String ID) {
        this.rawOffset = rawOffset;
        this.ID = ID;
    }
    
    public int getRawOffset() {
        return rawOffset;
    }
    
    public boolean useDaylightTime() {
        return false;
    }
    
    public String getID() {
        return ID;
    }
}
