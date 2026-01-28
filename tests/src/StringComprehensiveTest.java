public class StringComprehensiveTest {
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

    public static void assertEquals(String description, String expected, String actual) {
        assertTrue(description, (expected == null && actual == null) || 
                            (expected != null && expected.equals(actual)));
    }

    public static void assertEquals(String description, int expected, int actual) {
        assertTrue(description, expected == actual);
    }

    public static void testConstructors() {
        System.out.println("\n--- String Constructors ---");

        String s1 = new String();
        assertEquals("Empty string length", 0, s1.length());

        String s2 = new String("Hello");
        assertEquals("String from String literal", "Hello", s2);

        char[] chars = {'H', 'e', 'l', 'l', 'o'};
        String s3 = new String(chars);
        assertEquals("String from char array", "Hello", s3);

        String s4 = new String(chars, 1, 3);
        assertEquals("String from char array with offset", "ell", s4);

        byte[] bytes = {72, 101, 108, 108, 111};
        String s5 = new String(bytes);
        assertEquals("String from byte array", "Hello", s5);

        String s6 = new String(bytes, 1, 3);
        assertEquals("String from byte array with offset", "ell", s6);

        String s7 = new String(s2);
        assertEquals("String copy constructor", "Hello", s7);
    }

    public static void testBasicOperations() {
        System.out.println("\n--- Basic Operations ---");

        String s = "Hello World";
        assertEquals("length()", 11, s.length());

        assertEquals("charAt(0)", 'H', s.charAt(0));
        assertEquals("charAt(6)", 'W', s.charAt(6));
        assertEquals("charAt(10)", 'd', s.charAt(10));

        try {
            s.charAt(11);
            assertTrue("charAt(out of bounds) should throw", false);
        } catch (IndexOutOfBoundsException e) {
            assertTrue("charAt(out of bounds) throws exception", true);
        }
    }

    public static void testSubstring() {
        System.out.println("\n--- Substring ---");

        String s = "Hello World";
        assertEquals("substring(6)", "World", s.substring(6));
        assertEquals("substring(0,5)", "Hello", s.substring(0, 5));
        assertEquals("substring(6,11)", "World", s.substring(6, 11));
        assertEquals("substring(0,11)", "Hello World", s.substring(0, 11));
    }

    public static void testEquality() {
        System.out.println("\n--- Equality ---");

        String s1 = "Hello";
        String s2 = new String("Hello");
        String s3 = "World";
        String s4 = "Hello";

        assertTrue("s1.equals(s2)", s1.equals(s2));
        assertTrue("s1.equals(s4)", s1.equals(s4));
        assertTrue("!s1.equals(s3)", !s1.equals(s3));
        assertTrue("!s1.equals(null)", !s1.equals(null));
    }

    public static void testStartsWithEndsWith() {
        System.out.println("\n--- StartsWith/EndsWith ---");

        String s = "Hello World";
        assertTrue("startsWith(\"Hello\")", s.startsWith("Hello"));
        assertTrue("startsWith(\"H\")", s.startsWith("H"));
        assertTrue("!startsWith(\"World\")", !s.startsWith("World"));

        assertTrue("endsWith(\"World\")", s.endsWith("World"));
        assertTrue("endsWith(\"d\")", s.endsWith("d"));
        assertTrue("!endsWith(\"Hello\")", !s.endsWith("Hello"));
    }

    public static void testIndexOf() {
        System.out.println("\n--- IndexOf ---");

        String s = "Hello World";
        assertEquals("indexOf('H')", 0, s.indexOf('H'));
        assertEquals("indexOf('o')", 4, s.indexOf('o'));
        assertEquals("indexOf('x')", -1, s.indexOf('x'));

        assertEquals("indexOf(\"World\")", 6, s.indexOf("World"));
        assertEquals("indexOf(\"Hello\")", 0, s.indexOf("Hello"));
        assertEquals("indexOf(\"xyz\")", -1, s.indexOf("xyz"));

        assertEquals("indexOf('o', 5)", 7, s.indexOf('o', 5));
    }

    public static void testContains() {
        System.out.println("\n--- Contains ---");

        String s = "Hello World";
        assertTrue("contains(\"World\")", s.indexOf("World") >= 0);
        assertTrue("!contains(\"xyz\")", s.indexOf("xyz") == -1);
    }

    public static void testReplace() {
        System.out.println("\n--- Replace ---");

        String s = "Hello World";
        String s2 = s.replace('o', 'a');
        assertEquals("replace('o', 'a')", "Hella Warld", s2);

        String s3 = s.replace("World", "Java");
        assertEquals("replace(\"World\", \"Java\")", "Hello Java", s3);
    }

    public static void testToUpperCaseLowerCase() {
        System.out.println("\n--- ToUpperCase/ToLowerCase ---");

        String s1 = "Hello";
        String s2 = s1.toUpperCase();
        String s3 = s2.toLowerCase();

        assertEquals("toUpperCase()", "HELLO", s2);
        assertEquals("toLowerCase()", "hello", s3);
    }

    public static void testTrim() {
        System.out.println("\n--- Trim ---");

        String s1 = "  Hello  ";
        String s2 = s1.trim();
        assertEquals("trim()", "Hello", s2);

        String s3 = "\tHello\t";
        String s4 = s3.trim();
        assertEquals("trim() with tabs", "Hello", s4);
    }

    public static void testConcat() {
        System.out.println("\n--- Concat ---");

        String s1 = "Hello";
        String s2 = " World";
        String s3 = s1.concat(s2);
        assertEquals("concat()", "Hello World", s3);
    }

    public static void testValueOf() {
        System.out.println("\n--- ValueOf ---");

        assertEquals("valueOf(42)", "42", String.valueOf(42));
        assertEquals("valueOf(-42)", "-42", String.valueOf(-42));
        assertEquals("valueOf(true)", "true", String.valueOf(true));
        assertEquals("valueOf('A')", "A", String.valueOf('A'));
    }

    public static void testGetBytes() {
        System.out.println("\n--- GetBytes ---");

        String s = "ABC";
        byte[] bytes = s.getBytes();
        assertEquals("bytes length", 3, bytes.length);
        assertEquals("bytes[0]", (byte)'A', bytes[0]);
        assertEquals("bytes[1]", (byte)'B', bytes[1]);
        assertEquals("bytes[2]", (byte)'C', bytes[2]);
    }

    public static void testToCharArray() {
        System.out.println("\n--- ToCharArray ---");

        String s = "Hello";
        char[] chars = s.toCharArray();
        assertEquals("chars length", 5, chars.length);
        assertEquals("chars[0]", 'H', chars[0]);
        assertEquals("chars[4]", 'o', chars[4]);
    }

    public static void testEmptyAndNull() {
        System.out.println("\n--- Empty String ---");

        String s1 = "";
        assertEquals("Empty string length", 0, s1.length());
        assertTrue("Empty string equals()", s1.equals(""));

        String s2 = "   ".trim();
        assertEquals("Trimmed whitespace length", 0, s2.length());
    }

    public static void main(String[] args) {
        System.out.println("=== String Comprehensive Test ===");

        testConstructors();
        testBasicOperations();
        testSubstring();
        testEquality();
        testStartsWithEndsWith();
        testIndexOf();
        testContains();
        testReplace();
        testToUpperCaseLowerCase();
        testTrim();
        testConcat();
        testValueOf();
        testGetBytes();
        testToCharArray();
        testEmptyAndNull();

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
