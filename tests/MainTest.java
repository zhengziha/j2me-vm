public class MainTest {
    public static void main(String[] args) {
        System.out.println("Hello from main method!");
        System.out.println("Testing primitive types:");
        
        byte b = 127;
        short s = 32767;
        int i = 2147483647;
        long l = 9223372036854775807L;
        float f = 3.14159f;
        double d = 2.71828;
        char c = 'A';
        boolean bool = true;
        
        System.out.println("byte: " + b);
        System.out.println("short: " + s);
        System.out.println("int: " + i);
        System.out.println("long: " + l);
        System.out.println("float: " + f);
        System.out.println("double: " + d);
        System.out.println("char: " + c);
        System.out.println("boolean: " + bool);
    }
}