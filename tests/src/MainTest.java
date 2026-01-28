public class MainTest {
    private static int totalPassed = 0;
    private static int totalFailed = 0;
    private static int totalTestCount = 0;

    public static void runTest(String testName, TestRunner runner) {
        System.out.println("\n========================================");
        System.out.println("Running: " + testName);
        System.out.println("========================================");
        
        int beforePassed = totalPassed;
        int beforeFailed = totalFailed;
        
        runner.run();
        
        int afterPassed = totalPassed;
        int afterFailed = totalFailed;
        
        System.out.println("\n" + testName + " completed: " + (afterPassed - beforePassed) + " passed, " + (afterFailed - beforeFailed) + " failed");
    }

    interface TestRunner {
        void run();
    }

    public static void main(String[] args) {
        System.out.println("========================================");
        System.out.println("J2ME VM Comprehensive Test Suite");
        System.out.println("========================================");

        runTest("I/O Streams Test", new TestRunner() {
            public void run() {
                ComprehensiveIOTest.main(new String[]{});
                totalPassed += ComprehensiveIOTest.getPassed();
                totalFailed += ComprehensiveIOTest.getFailed();
            }
        });

        runTest("Wrapper Classes Test", new TestRunner() {
            public void run() {
                WrapperClassesTest.main(new String[]{});
                totalPassed += WrapperClassesTest.getPassed();
                totalFailed += WrapperClassesTest.getFailed();
            }
        });

        runTest("String Comprehensive Test", new TestRunner() {
            public void run() {
                StringComprehensiveTest.main(new String[]{});
                totalPassed += StringComprehensiveTest.getPassed();
                totalFailed += StringComprehensiveTest.getFailed();
            }
        });

        runTest("Math Comprehensive Test", new TestRunner() {
            public void run() {
                MathComprehensiveTest.main(new String[]{});
                totalPassed += MathComprehensiveTest.getPassed();
                totalFailed += MathComprehensiveTest.getFailed();
            }
        });

        runTest("Collection Comprehensive Test", new TestRunner() {
            public void run() {
                CollectionComprehensiveTest.main(new String[]{});
                totalPassed += CollectionComprehensiveTest.getPassed();
                totalFailed += CollectionComprehensiveTest.getFailed();
            }
        });

        totalTestCount = totalPassed + totalFailed;

        System.out.println("\n========================================");
        System.out.println("FINAL TEST SUMMARY");
        System.out.println("========================================");
        System.out.println("Total Passed: " + totalPassed);
        System.out.println("Total Failed: " + totalFailed);
        System.out.println("Total Tests: " + totalTestCount);
        System.out.println("Success Rate: " + (totalTestCount > 0 ? (totalPassed * 100 / totalTestCount) : 0) + "%");
        System.out.println("========================================");

        if (totalFailed == 0) {
            System.out.println("ALL TESTS PASSED!");
            System.out.println("========================================");
        } else {
            System.out.println("SOME TESTS FAILED!");
            System.out.println("========================================");
        }
    }
}
