public class ExceptionTest {
    public static void main(String[] args) {
        System.out.println("=== Exception Handling Test ===");
        
        testBasicException();
        testTryCatchFinally();
        testMultipleCatch();
        testNestedTryCatch();
        testCustomException();
        
        System.out.println("=== All Exception Tests Completed ===");
    }
    
    static void testBasicException() {
        System.out.println("\n--- Basic Exception Test ---");
        
        try {
            System.out.println("Attempting to divide by zero...");
            int result = 10 / 0;
            System.out.println("Result: " + result);
            System.out.println("Basic exception: FAILED (no exception thrown)");
        } catch (ArithmeticException e) {
            System.out.println("Caught ArithmeticException: " + e.getMessage());
            System.out.println("Basic exception: PASSED");
        }
    }
    
    static void testTryCatchFinally() {
        System.out.println("\n--- Try-Catch-Finally Test ---");
        
        boolean exceptionCaught = false;
        boolean finallyExecuted = false;
        
        try {
            System.out.println("In try block");
            int[] array = new int[5];
            array[10] = 100;
        } catch (ArrayIndexOutOfBoundsException e) {
            System.out.println("Caught ArrayIndexOutOfBoundsException: " + e.getMessage());
            exceptionCaught = true;
        } finally {
            System.out.println("In finally block");
            finallyExecuted = true;
        }
        
        if (exceptionCaught && finallyExecuted) {
            System.out.println("Try-catch-finally: PASSED");
        } else {
            System.out.println("Try-catch-finally: FAILED");
        }
    }
    
    static void testMultipleCatch() {
        System.out.println("\n--- Multiple Catch Test ---");
        
        try {
            System.out.println("Testing string to integer conversion...");
            String str = "not a number";
            int num = Integer.parseInt(str);
            System.out.println("Number: " + num);
        } catch (NumberFormatException e) {
            System.out.println("Caught NumberFormatException: " + e.getMessage());
            System.out.println("Multiple catch: PASSED");
        } catch (Exception e) {
            System.out.println("Caught generic Exception: " + e.getMessage());
            System.out.println("Multiple catch: FAILED (caught wrong exception)");
        }
    }
    
    static void testNestedTryCatch() {
        System.out.println("\n--- Nested Try-Catch Test ---");
        
        boolean outerCaught = false;
        boolean innerCaught = false;
        
        try {
            System.out.println("Outer try block");
            try {
                System.out.println("Inner try block");
                String s = null;
                int length = s.length();
            } catch (NullPointerException e) {
                System.out.println("Inner catch: " + e.getMessage());
                innerCaught = true;
                throw new RuntimeException("Re-throwing from inner catch");
            }
        } catch (RuntimeException e) {
            System.out.println("Outer catch: " + e.getMessage());
            outerCaught = true;
        }
        
        if (innerCaught && outerCaught) {
            System.out.println("Nested try-catch: PASSED");
        } else {
            System.out.println("Nested try-catch: FAILED");
        }
    }
    
    static void testCustomException() {
        System.out.println("\n--- Custom Exception Test ---");
        
        try {
            System.out.println("Testing custom exception...");
            validateAge(15);
            System.out.println("Custom exception: FAILED (no exception thrown)");
        } catch (InvalidAgeException e) {
            System.out.println("Caught InvalidAgeException: " + e.getMessage());
            System.out.println("Custom exception: PASSED");
        }
        
        try {
            System.out.println("Testing valid age...");
            validateAge(25);
            System.out.println("Age validation passed");
        } catch (InvalidAgeException e) {
            System.out.println("Custom exception: FAILED (exception thrown for valid age)");
        }
    }
    
    static void validateAge(int age) throws InvalidAgeException {
        if (age < 18) {
            throw new InvalidAgeException("Age must be at least 18, but was: " + age);
        }
    }
    
    static class InvalidAgeException extends Exception {
        public InvalidAgeException(String message) {
            super(message);
        }
    }
}
