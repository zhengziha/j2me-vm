package java.util;

public abstract class Calendar {
    public static final int YEAR = 1;
    public static final int MONTH = 2;
    public static final int DATE = 5;
    public static final int DAY_OF_MONTH = 5;
    public static final int DAY_OF_WEEK = 7;
    public static final int AMPM = 9;
    public static final int HOUR = 10;
    public static final int HOUR_OF_DAY = 11;
    public static final int MINUTE = 12;
    public static final int SECOND = 13;
    public static final int MILLISECOND = 14;
    
    public static final int SUNDAY = 1;
    public static final int MONDAY = 2;
    public static final int TUESDAY = 3;
    public static final int WEDNESDAY = 4;
    public static final int THURSDAY = 5;
    public static final int FRIDAY = 6;
    public static final int SATURDAY = 7;
    
    public static final int JANUARY = 0;
    public static final int FEBRUARY = 1;
    public static final int MARCH = 2;
    public static final int APRIL = 3;
    public static final int MAY = 4;
    public static final int JUNE = 5;
    public static final int JULY = 6;
    public static final int AUGUST = 7;
    public static final int SEPTEMBER = 8;
    public static final int OCTOBER = 9;
    public static final int NOVEMBER = 10;
    public static final int DECEMBER = 11;
    
    public static final int AM = 0;
    public static final int PM = 1;

    protected long time;
    protected int[] fields;
    protected boolean[] isSet;
    
    protected Calendar() {
        this(TimeZone.getDefault());
    }
    
    protected Calendar(TimeZone zone) {
        fields = new int[20];
        isSet = new boolean[20];
    }

    public static Calendar getInstance() {
        return getInstance(TimeZone.getDefault());
    }

    public static Calendar getInstance(TimeZone zone) {
        return new GregorianCalendar(zone);
    }
    
    public final Date getTime() {
        return new Date(time);
    }
    
    public final void setTime(Date date) {
        setTimeInMillis(date.getTime());
    }
    
    public final void setTimeInMillis(long millis) {
        time = millis;
        computeFields();
    }
    
    public final long getTimeInMillis() {
        return time;
    }
    
    public final int get(int field) {
        return fields[field];
    }
    
    public final void set(int field, int value) {
        fields[field] = value;
        isSet[field] = true;
        computeTime();
    }
    
    protected abstract void computeFields();
    protected abstract void computeTime();
}

class GregorianCalendar extends Calendar {
    public GregorianCalendar(TimeZone zone) {
        super(zone);
        setTimeInMillis(System.currentTimeMillis());
    }
    
    protected void computeFields() {
        // Very simplified, just for stub compilation/basic run
        // Real implementation requires complex date math
        fields[YEAR] = 2023;
        fields[MONTH] = JANUARY;
        fields[DAY_OF_MONTH] = 1;
    }
    
    protected void computeTime() {
        // Simplified
    }
}
