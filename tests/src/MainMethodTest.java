public class MainMethodTest {
    public static void main(String[] args) {
        System.out.println("Hello from MainMethodTest!");
        System.out.println("Args count: " + args.length);
        for (int i = 0; i < args.length; i++) {
            System.out.println("Arg[" + i + "]: " + args[i]);
        }
    }
}
