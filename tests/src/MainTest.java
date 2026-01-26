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

        if (b == 127) System.out.println("byte test passed");
        if (s == 32767) System.out.println("short test passed");
        if (i == 2147483647) System.out.println("int test passed");
        if (l == 9223372036854775807L) System.out.println("long test passed");
        if (f == 3.14159f) System.out.println("float test passed");
        if (d == 2.71828) System.out.println("double test passed");
        if (c == 'A') System.out.println("char test passed");
        if (bool == true) System.out.println("boolean test passed");

        if (1+1==2){
            System.out.println("1+1=2 test passed");
        }
    }
}