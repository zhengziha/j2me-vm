package java.lang;

public class String {
    private char[] value;
    private int offset;
    private int count;

    public String() {
        this.value = new char[0];
    }

    public String(byte[] data) {
        this(data, 0, data.length);
    }

    public String(byte[] data, int offset, int length) {
        // Simplified: assume default encoding (ISO-8859-1 or similar)
        this.value = new char[length];
        this.count = length;
        for (int i = 0; i < length; i++) {
            this.value[i] = (char)(data[offset + i] & 0xff);
        }
    }
    
    public String(char[] data) {
        this(data, 0, data.length);
    }
    
    public String(char[] data, int offset, int length) {
        this.value = new char[length];
        this.count = length;
        System.arraycopy(data, offset, this.value, 0, length);
    }
    
    public String(String original) {
        this.value = original.value;
        this.offset = original.offset;
        this.count = original.count;
    }

    public int length() {
        return count;
    }

    public char charAt(int index) {
        if (index < 0 || index >= count) throw new IndexOutOfBoundsException();
        return value[offset + index];
    }

    public String substring(int beginIndex) {
        return substring(beginIndex, count);
    }

    public String substring(int beginIndex, int endIndex) {
        if (beginIndex < 0 || endIndex > count || beginIndex > endIndex) {
            throw new IndexOutOfBoundsException();
        }
        if (beginIndex == 0 && endIndex == count) return this;
        // In full Java, String shares char array. In simplified stubs, we can copy.
        return new String(value, offset + beginIndex, endIndex - beginIndex);
    }

    public byte[] getBytes() {
        // Simplified default encoding
        byte[] b = new byte[count];
        for (int i = 0; i < count; i++) {
            b[i] = (byte)value[offset + i];
        }
        return b;
    }
    
    public char[] toCharArray() {
        char[] result = new char[count];
        System.arraycopy(value, offset, result, 0, count);
        return result;
    }

    public static String valueOf(int i) {
        return Integer.toString(i);
    }

    public static String valueOf(long l) {
        return Long.toString(l);
    }

    public static String valueOf(float f) {
        return Float.toString(f);
    }

    public static String valueOf(double d) {
        return Double.toString(d);
    }

    public static String valueOf(char c) {
        return new String(new char[]{c});
    }

    public static String valueOf(boolean b) {
        return b ? "true" : "false";
    }
    
    public static String valueOf(Object obj) {
        return (obj == null) ? "null" : obj.toString();
    }
    
    public String toString() {
        return this;
    }
    
    public boolean equals(Object anObject) {
        if (this == anObject) {
            return true;
        }
        if (anObject instanceof String) {
            String anotherString = (String)anObject;
            int n = count;
            if (n == anotherString.count) {
                char v1[] = value;
                char v2[] = anotherString.value;
                int i = offset;
                int j = anotherString.offset;
                while (n-- != 0) {
                    if (v1[i++] != v2[j++])
                        return false;
                }
                return true;
            }
        }
        return false;
    }
    
    public int hashCode() {
        int h = 0;
        int off = offset;
        char val[] = value;
        int len = count;

        for (int i = 0; i < len; i++) {
            h = 31*h + val[off++];
        }
        return h;
    }
    
    public int indexOf(int ch) {
        return indexOf(ch, 0);
    }
    
    public int indexOf(int ch, int fromIndex) {
        int max = offset + count;
        char v[] = value;
        if (fromIndex < 0) {
            fromIndex = 0;
        } else if (fromIndex >= count) {
            // Note: fromIndex might be near -1>>>1.
            return -1;
        }

        int i = offset + fromIndex;
        if (ch < Character.MIN_SUPPLEMENTARY_CODE_POINT) {
            // handle most cases here (ch is a BMP code point or a
            // negative value (invalid code point))
            for (; i < max ; i++) {
                if (v[i] == ch) {
                    return i - offset;
                }
            }
            return -1;
        }
        return -1;
    }

    public boolean startsWith(String prefix) {
        return startsWith(prefix, 0);
    }

    public boolean startsWith(String prefix, int toffset) {
        char ta[] = value;
        int to = offset + toffset;
        char pa[] = prefix.value;
        int po = prefix.offset;
        int pc = prefix.count;
        // Note: toffset might be near -1>>>1.
        if ((toffset < 0) || (toffset > count - pc)) {
            return false;
        }
        while (--pc >= 0) {
            if (ta[to++] != pa[po++]) {
                return false;
            }
        }
        return true;
    }

    public String trim() {
        int len = count;
        int st = 0;
        int off = offset;      /* avoid getfield opcode */
        char[] val = value;    /* avoid getfield opcode */

        while ((st < len) && (val[off + st] <= ' ')) {
            st++;
        }
        while ((st < len) && (val[off + len - 1] <= ' ')) {
            len--;
        }
        return ((st > 0) || (len < count)) ? substring(st, len) : this;
    }
}
