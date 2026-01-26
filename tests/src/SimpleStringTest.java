public class SimpleStringTest {
    public static void main(String[] args) {
        System.out.println("=== Simple String Test ===");
        
        String s1 = "Hello";
        String s2 = "World";
        
        System.out.println("s1 = " + s1);
        System.out.println("s2 = " + s2);
        
        StringBuilder sb = new StringBuilder();
        sb.append(s1);
        System.out.println("After append s1: " + sb.toString());
        
        sb.append(" ");
        System.out.println("After append space: " + sb.toString());
        
        sb.append(s2);
        System.out.println("After append s2: " + sb.toString());
        
        String result = s1 + " " + s2;
        System.out.println("Direct concatenation: " + result);
    }
}
