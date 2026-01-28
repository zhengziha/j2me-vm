public class TestArgsMain {
    public static void main(String[] args) {
        System.out.println("TestArgsMain started");
        System.out.println("Args count: " + args.length);
        for (int i = 0; i < args.length; i++) {
            System.out.println("arg[" + i + "] = " + args[i]);
        }
        System.out.println("TestArgsMain completed");
    }
}
