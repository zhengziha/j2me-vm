public class DebugStringTest {
    public static void main(String[] args) {
        System.out.println("=== Debug String Test ===");
        
        String s1 = "Hello";
        System.out.println("s1 = " + s1);
        
        StringBuilder sb = new StringBuilder();
        sb.append("Test");
        String result = sb.toString();
        System.out.println("Direct sb.toString: " + result);
        
        System.out.println("After sb.toString");
    }
}
