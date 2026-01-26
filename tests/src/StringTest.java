public class StringTest {
    public static void main(String[] args) {
        System.out.println("=== String Operations Test ===");
        
        testStringCreation();
        testStringConcatenation();
        testStringComparison();
        testStringMethods();
        testStringBuilder();
        
        System.out.println("=== All String Tests Completed ===");
    }
    
    static void testStringCreation() {
        System.out.println("\n--- String Creation Test ---");
        
        String s1 = "Hello";
        String s2 = new String("World");
        String s3 = new String(new char[] {'J', '2', 'M', 'E'});
        
        System.out.println("s1 = " + s1);
        System.out.println("s2 = " + s2);
        System.out.println("s3 = " + s3);
        
        if (s1.equals("Hello")) {
            System.out.println("String literal creation: PASSED");
        } else {
            System.out.println("String literal creation: FAILED");
        }
        
        if (s2.equals("World")) {
            System.out.println("String constructor: PASSED");
        } else {
            System.out.println("String constructor: FAILED");
        }
        
        if (s3.equals("J2ME")) {
            System.out.println("String from char array: PASSED");
        } else {
            System.out.println("String from char array: FAILED");
        }
    }
    
    static void testStringConcatenation() {
        System.out.println("\n--- String Concatenation Test ---");
        
        String s1 = "Hello";
        String s2 = " ";
        String s3 = "World";
        String result = s1 + s2 + s3;
        
        System.out.println("Concatenation result: " + result);
        
        if (result.equals("Hello World")) {
            System.out.println("String concatenation: PASSED");
        } else {
            System.out.println("String concatenation: FAILED");
        }
        
        String s4 = "Number: " + 42;
        System.out.println("String with number: " + s4);
        
        if (s4.equals("Number: 42")) {
            System.out.println("String with number: PASSED");
        } else {
            System.out.println("String with number: FAILED");
        }
    }
    
    static void testStringComparison() {
        System.out.println("\n--- String Comparison Test ---");
        
        String s1 = "Hello";
        String s2 = "Hello";
        String s3 = "World";
        
        boolean equals1 = s1.equals(s2);
        boolean equals2 = s1.equals(s3);
        
        System.out.println("s1.equals(s2): " + equals1);
        System.out.println("s1.equals(s3): " + equals2);
        
        if (equals1 && !equals2) {
            System.out.println("String equals: PASSED");
        } else {
            System.out.println("String equals: FAILED");
        }
        
        int cmp1 = s1.compareTo(s2);
        int cmp2 = s1.compareTo(s3);
        
        System.out.println("s1.compareTo(s2): " + cmp1);
        System.out.println("s1.compareTo(s3): " + cmp2);
        
        if (cmp1 == 0 && cmp2 < 0) {
            System.out.println("String compareTo: PASSED");
        } else {
            System.out.println("String compareTo: FAILED");
        }
    }
    
    static void testStringMethods() {
        System.out.println("\n--- String Methods Test ---");
        
        String s = "Hello J2ME World";
        
        int length = s.length();
        System.out.println("Length: " + length);
        
        if (length == 16) {
            System.out.println("String length: PASSED");
        } else {
            System.out.println("String length: FAILED");
        }
        
        char firstChar = s.charAt(0);
        System.out.println("First char: " + firstChar);
        
        if (firstChar == 'H') {
            System.out.println("String charAt: PASSED");
        } else {
            System.out.println("String charAt: FAILED");
        }
        
        String substring = s.substring(6, 10);
        System.out.println("Substring(6,10): " + substring);
        
        if (substring.equals("J2ME")) {
            System.out.println("String substring: PASSED");
        } else {
            System.out.println("String substring: FAILED");
        }
        
        String upper = s.toUpperCase();
        String lower = s.toLowerCase();
        System.out.println("Uppercase: " + upper);
        System.out.println("Lowercase: " + lower);
        
        if (upper.equals("HELLO J2ME WORLD") && lower.equals("hello j2me world")) {
            System.out.println("String toUpperCase/toLowerCase: PASSED");
        } else {
            System.out.println("String toUpperCase/toLowerCase: FAILED");
        }
        
        String trimmed = "  Hello  ".trim();
        System.out.println("Trimmed: '" + trimmed + "'");
        
        if (trimmed.equals("Hello")) {
            System.out.println("String trim: PASSED");
        } else {
            System.out.println("String trim: FAILED");
        }
        
        boolean startsWith = s.startsWith("Hello");
        boolean endsWith = s.endsWith("World");
        boolean contains = s.contains("J2ME");
        
        System.out.println("Starts with 'Hello': " + startsWith);
        System.out.println("Ends with 'World': " + endsWith);
        System.out.println("Contains 'J2ME': " + contains);
        
        if (startsWith && endsWith && contains) {
            System.out.println("String startsWith/endsWith/contains: PASSED");
        } else {
            System.out.println("String startsWith/endsWith/contains: FAILED");
        }
        
        String replaced = s.replace("J2ME", "Java");
        System.out.println("Replaced: " + replaced);
        
        if (replaced.equals("Hello Java World")) {
            System.out.println("String replace: PASSED");
        } else {
            System.out.println("String replace: FAILED");
        }
    }
    
    static void testStringBuilder() {
        System.out.println("\n--- StringBuilder Test ---");
        
        StringBuilder sb = new StringBuilder();
        sb.append("Hello");
        sb.append(" ");
        sb.append("J2ME");
        
        String result = sb.toString();
        System.out.println("StringBuilder result: " + result);
        
        if (result.equals("Hello J2ME")) {
            System.out.println("StringBuilder append: PASSED");
        } else {
            System.out.println("StringBuilder append: FAILED");
        }
        
        sb.insert(5, " Beautiful");
        result = sb.toString();
        System.out.println("After insert: " + result);
        
        if (result.equals("Hello Beautiful J2ME")) {
            System.out.println("StringBuilder insert: PASSED");
        } else {
            System.out.println("StringBuilder insert: FAILED");
        }
        
        sb.delete(5, 15);
        result = sb.toString();
        System.out.println("After delete: " + result);
        
        if (result.equals("Hello J2ME")) {
            System.out.println("StringBuilder delete: PASSED");
        } else {
            System.out.println("StringBuilder delete: FAILED");
        }
        
        sb.reverse();
        result = sb.toString();
        System.out.println("After reverse: " + result);
        
        if (result.equals("EM2J olleH")) {
            System.out.println("StringBuilder reverse: PASSED");
        } else {
            System.out.println("StringBuilder reverse: FAILED");
        }
    }
}
