public class WrapperClassesTest {
    private static int passed = 0;
    private static int failed = 0;

    public static int getPassed() {
        return passed;
    }

    public static int getFailed() {
        return failed;
    }

    public static void assertTrue(String description, boolean condition) {
        if (condition) {
            System.out.println("  [PASS] " + description);
            passed++;
        } else {
            System.out.println("  [FAIL] " + description);
            failed++;
        }
    }

    public static void assertEquals(String description, int expected, int actual) {
        assertTrue(description, expected == actual);
    }

    public static void assertEquals(String description, long expected, long actual) {
        assertTrue(description, expected == actual);
    }

    public static void assertEquals(String description, boolean expected, boolean actual) {
        assertTrue(description, expected == actual);
    }

    public static void testInteger() {
        System.out.println("\n--- Integer Tests ---");

        assertEquals("MIN_VALUE", 0x80000000, Integer.MIN_VALUE);
        assertEquals("MAX_VALUE", 0x7fffffff, Integer.MAX_VALUE);

        Integer i = new Integer(42);
        assertEquals("intValue()", 42, i.intValue());

        Integer i2 = new Integer(42);
        assertTrue("equals()", i.equals(i2));
        assertTrue("hashCode() same value", i.hashCode() == i2.hashCode());

        Integer i3 = new Integer(43);
        assertTrue("equals() different value", !i.equals(i3));

        assertEquals("parseInt(42)", 42, Integer.parseInt("42"));
        assertEquals("parseInt(-42)", -42, Integer.parseInt("-42"));
        assertEquals("parseInt(0)", 0, Integer.parseInt("0"));

        String s = i.toString();
        assertTrue("toString() contains '42'", s.indexOf("42") >= 0);
    }

    public static void testLong() {
        System.out.println("\n--- Long Tests ---");

        assertEquals("MIN_VALUE", 0x8000000000000000L, Long.MIN_VALUE);
        assertEquals("MAX_VALUE", 0x7fffffffffffffffL, Long.MAX_VALUE);

        Long l = new Long(42L);
        assertEquals("longValue()", 42L, l.longValue());

        Long l2 = new Long(42L);
        assertTrue("equals()", l.equals(l2));
    }

    public static void testShort() {
        System.out.println("\n--- Short Tests ---");

        assertEquals("MIN_VALUE", (short)0x8000, Short.MIN_VALUE);
        assertEquals("MAX_VALUE", (short)0x7fff, Short.MAX_VALUE);

        Short s = new Short((short)42);
        assertEquals("shortValue()", (short)42, s.shortValue());
    }

    public static void testByte() {
        System.out.println("\n--- Byte Tests ---");

        assertEquals("MIN_VALUE", (byte)0x80, Byte.MIN_VALUE);
        assertEquals("MAX_VALUE", (byte)0x7f, Byte.MAX_VALUE);

        Byte b = new Byte((byte)42);
        assertEquals("byteValue()", (byte)42, b.byteValue());
    }

    public static void testBoolean() {
        System.out.println("\n--- Boolean Tests ---");

        Boolean b1 = new Boolean(true);
        assertEquals("booleanValue(true)", true, b1.booleanValue());

        Boolean b2 = new Boolean(false);
        assertEquals("booleanValue(false)", false, b2.booleanValue());

        assertTrue("TRUE value", Boolean.TRUE.booleanValue());
        assertTrue("FALSE value", !Boolean.FALSE.booleanValue());
    }

    public static void testCharacter() {
        System.out.println("\n--- Character Tests ---");

        assertEquals("MIN_VALUE", '\u0000', Character.MIN_VALUE);
        assertEquals("MAX_VALUE", '\uffff', Character.MAX_VALUE);

        Character c = new Character('A');
        assertEquals("charValue()", 'A', c.charValue());

        Character c2 = new Character('A');
        assertTrue("equals()", c.equals(c2));

        assertTrue("isDigit('0')", Character.isDigit('0'));
        assertTrue("!isDigit('A')", !Character.isDigit('A'));

        assertTrue("isLetter('A')", Character.isLetter('A'));
        assertTrue("!isLetter('0')", !Character.isLetter('0'));

        assertTrue("isUpperCase('A')", Character.isUpperCase('A'));
        assertTrue("!isUpperCase('a')", !Character.isUpperCase('a'));

        assertTrue("isLowerCase('a')", Character.isLowerCase('a'));
        assertTrue("!isLowerCase('A')", !Character.isLowerCase('A'));

        assertEquals("toLowerCase('A')", 'a', Character.toLowerCase('A'));
        assertEquals("toUpperCase('a')", 'A', Character.toUpperCase('a'));
    }

    public static void testFloat() {
        System.out.println("\n--- Float Tests ---");

        Float f = new Float(3.14f);
        assertTrue("floatValue()", Math.abs(f.floatValue() - 3.14f) < 0.001f);

        Float f2 = new Float(3.14f);
        assertTrue("equals()", f.equals(f2));

        assertTrue("isNaN(Float.NaN)", Float.isNaN(Float.NaN));
        assertTrue("!isNaN(1.0f)", !Float.isNaN(1.0f));

        assertTrue("isInfinite(Float.POSITIVE_INFINITY)", Float.isInfinite(Float.POSITIVE_INFINITY));
        assertTrue("isInfinite(Float.NEGATIVE_INFINITY)", Float.isInfinite(Float.NEGATIVE_INFINITY));
    }

    public static void testDouble() {
        System.out.println("\n--- Double Tests ---");

        Double d = new Double(3.14159);
        assertTrue("doubleValue()", Math.abs(d.doubleValue() - 3.14159) < 0.0001);

        Double d2 = new Double(3.14159);
        assertTrue("equals()", d.equals(d2));

        assertTrue("isNaN(Double.NaN)", Double.isNaN(Double.NaN));
        assertTrue("!isNaN(1.0)", !Double.isNaN(1.0));

        assertTrue("isInfinite(Double.POSITIVE_INFINITY)", Double.isInfinite(Double.POSITIVE_INFINITY));
        assertTrue("isInfinite(Double.NEGATIVE_INFINITY)", Double.isInfinite(Double.NEGATIVE_INFINITY));
    }

    public static void main(String[] args) {
        System.out.println("=== Wrapper Classes Test ===");

        testInteger();
        testLong();
        testShort();
        testByte();
        testBoolean();
        testCharacter();
        testFloat();
        testDouble();

        System.out.println("\n=== Test Summary ===");
        System.out.println("Passed: " + passed);
        System.out.println("Failed: " + failed);
        System.out.println("Total: " + (passed + failed));

        if (failed == 0) {
            System.out.println("ALL TESTS PASSED!");
        } else {
            System.out.println("SOME TESTS FAILED!");
        }
    }
}
