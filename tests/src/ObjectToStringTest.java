public class ObjectToStringTest {
    public static void main(String[] args) {
        System.out.println("=== Object.toString() Test ===");
        
        Object obj = new Object();
        System.out.println("Object.toString(): " + obj.toString());
        
        String str = new String("Hello");
        System.out.println("String.toString(): " + str.toString());
        
        StringBuilder sb = new StringBuilder();
        System.out.println("StringBuilder.toString(): " + sb.toString());
    }
}
