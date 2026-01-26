public class MathTest {
    public static void main(String[] args) {
        System.out.println("Testing Float and Double methods:");
        
        float f = 3.14159f;
        System.out.println("Original float: " + f);
        
        int bits = Float.floatToIntBits(f);
        System.out.println("Float to int bits: " + bits);
        
        float f2 = Float.intBitsToFloat(bits);
        System.out.println("Int bits to float: " + f2);
        
        System.out.println();
        
        double d = 2.71828;
        System.out.println("Original double: " + d);
        
        long lbits = Double.doubleToLongBits(d);
        System.out.println("Double to long bits: " + lbits);
        
        double d2 = Double.longBitsToDouble(lbits);
        System.out.println("Long bits to double: " + d2);
        
        System.out.println();
        System.out.println("Testing Math methods:");
        
        System.out.println("sin(0) = " + Math.sin(0));
        System.out.println("cos(0) = " + Math.cos(0));
        System.out.println("tan(0) = " + Math.tan(0));
        System.out.println("sqrt(16) = " + Math.sqrt(16));
        System.out.println("ceil(3.7) = " + Math.ceil(3.7));
        System.out.println("floor(3.7) = " + Math.floor(3.7));
    }
}
