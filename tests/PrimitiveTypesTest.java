public class PrimitiveTypesTest {
    byte byteField;
    short shortField;
    int intField;
    long longField;
    float floatField;
    double doubleField;
    char charField;
    boolean booleanField;

    public PrimitiveTypesTest() {
        byteField = 127;
        shortField = 32767;
        intField = 2147483647;
        longField = 9223372036854775807L;
        floatField = 3.14159f;
        doubleField = 2.718281828459045;
        charField = 'A';
        booleanField = true;
    }

    public static void main(String[] args) {
        PrimitiveTypesTest test = new PrimitiveTypesTest();
        
        System.out.println("byteField = " + test.byteField);
        System.out.println("shortField = " + test.shortField);
        System.out.println("intField = " + test.intField);
        System.out.println("longField = " + test.longField);
        System.out.println("floatField = " + test.floatField);
        System.out.println("doubleField = " + test.doubleField);
        System.out.println("charField = " + test.charField);
        System.out.println("booleanField = " + test.booleanField);
    }
}
