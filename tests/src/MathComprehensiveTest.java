public class MathComprehensiveTest {
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

    public static void assertEquals(String description, double expected, double actual, double delta) {
        assertTrue(description, Math.abs(expected - actual) < delta);
    }

    public static void assertEquals(String description, int expected, int actual) {
        assertTrue(description, expected == actual);
    }

    public static void testConstants() {
        System.out.println("\n--- Math Constants ---");

        assertEquals("PI value", 3.141592653589793, Math.PI, 0.0000001);
        assertEquals("E value", 2.718281828459045, Math.E, 0.0000001);
    }

    public static void testAbs() {
        System.out.println("\n--- Math.abs() ---");

        assertEquals("abs(5)", 5, Math.abs(5));
        assertEquals("abs(-5)", 5, Math.abs(-5));
        assertEquals("abs(0)", 0, Math.abs(0));

        assertEquals("abs(5.5)", 5.5, Math.abs(5.5), 0.0001);
        assertEquals("abs(-5.5)", 5.5, Math.abs(-5.5), 0.0001);
    }

    public static void testMinMax() {
        System.out.println("\n--- Math.min()/Math.max() ---");

        assertEquals("min(5, 10)", 5, Math.min(5, 10));
        assertEquals("min(10, 5)", 5, Math.min(10, 5));
        assertEquals("min(5, 5)", 5, Math.min(5, 5));

        assertEquals("max(5, 10)", 10, Math.max(5, 10));
        assertEquals("max(10, 5)", 10, Math.max(10, 5));
        assertEquals("max(5, 5)", 5, Math.max(5, 5));

        assertEquals("min(5.5, 10.5)", 5.5, Math.min(5.5, 10.5), 0.0001);
        assertEquals("max(5.5, 10.5)", 10.5, Math.max(5.5, 10.5), 0.0001);
    }

    public static void testRound() {
        System.out.println("\n--- Math.round() ---");

        assertEquals("round(5.4)", 5, Math.round(5.4f));
        assertEquals("round(5.5)", 6, Math.round(5.5f));
        assertEquals("round(5.6)", 6, Math.round(5.6f));

        assertEquals("round(-5.4)", -5, Math.round(-5.4f));
        assertEquals("round(-5.5)", -5, Math.round(-5.5f));
        assertEquals("round(-5.6)", -6, Math.round(-5.6f));
    }

    public static void testFloorCeil() {
        System.out.println("\n--- Math.floor()/Math.ceil() ---");

        assertEquals("floor(5.7)", 5.0, Math.floor(5.7), 0.0001);
        assertEquals("floor(5.0)", 5.0, Math.floor(5.0), 0.0001);
        assertEquals("floor(-5.7)", -6.0, Math.floor(-5.7), 0.0001);

        assertEquals("ceil(5.7)", 6.0, Math.ceil(5.7), 0.0001);
        assertEquals("ceil(5.0)", 5.0, Math.ceil(5.0), 0.0001);
        assertEquals("ceil(-5.7)", -5.0, Math.ceil(-5.7), 0.0001);
    }

    public static void testSqrt() {
        System.out.println("\n--- Math.sqrt() ---");

        assertEquals("sqrt(4)", 2.0, Math.sqrt(4), 0.0001);
        assertEquals("sqrt(9)", 3.0, Math.sqrt(9), 0.0001);
        assertEquals("sqrt(16)", 4.0, Math.sqrt(16), 0.0001);
        assertEquals("sqrt(2)", 1.4142, Math.sqrt(2), 0.0001);
        assertEquals("sqrt(0)", 0.0, Math.sqrt(0), 0.0001);
    }

    public static void testPow() {
        System.out.println("\n--- Math.pow() ---");

        assertEquals("pow(2, 3)", 8.0, Math.pow(2, 3), 0.0001);
        assertEquals("pow(2, 0)", 1.0, Math.pow(2, 0), 0.0001);
        assertEquals("pow(2, 10)", 1024.0, Math.pow(2, 10), 0.001);
        assertEquals("pow(10, 2)", 100.0, Math.pow(10, 2), 0.0001);
    }

    public static void testSinCosTan() {
        System.out.println("\n--- Math.sin()/cos()/tan() ---");

        assertEquals("sin(0)", 0.0, Math.sin(0), 0.0001);
        assertEquals("cos(0)", 1.0, Math.cos(0), 0.0001);
        assertEquals("tan(0)", 0.0, Math.tan(0), 0.0001);
    }

    public static void testRandom() {
        System.out.println("\n--- Math.random() ---");

        double r = Math.random();
        assertTrue("random() >= 0", r >= 0.0);
        assertTrue("random() < 1", r < 1.0);

        double r2 = Math.random();
        assertTrue("random() >= 0", r2 >= 0.0);
        assertTrue("random() < 1", r2 < 1.0);

        int countPositive = 0;
        int countNegative = 0;
        int countZero = 0;
        for (int i = 0; i < 100; i++) {
            double val = Math.random() - 0.5;
            if (val > 0) countPositive++;
            else if (val < 0) countNegative++;
            else countZero++;
        }
        assertTrue("random() distribution (positive)", countPositive > 30);
        assertTrue("random() distribution (negative)", countNegative > 30);
    }

    public static void testToRadiansDegrees() {
        System.out.println("\n--- Math.toRadians()/toDegrees() ---");

        assertEquals("toRadians(180)", Math.PI, Math.toRadians(180), 0.0001);
        assertEquals("toRadians(90)", Math.PI / 2, Math.toRadians(90), 0.0001);
        assertEquals("toRadians(0)", 0.0, Math.toRadians(0), 0.0001);

        assertEquals("toDegrees(Math.PI)", 180.0, Math.toDegrees(Math.PI), 0.0001);
        assertEquals("toDegrees(Math.PI/2)", 90.0, Math.toDegrees(Math.PI / 2), 0.0001);
    }

    public static void main(String[] args) {
        System.out.println("=== Math Comprehensive Test ===");

        testConstants();
        testAbs();
        testMinMax();
        testRound();
        testFloorCeil();
        testSqrt();
        testPow();
        testSinCosTan();
        testRandom();
        testToRadiansDegrees();

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
