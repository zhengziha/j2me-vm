public class ArrayTest {
    public static void main(String[] args) {
        System.out.println("=== Array Operations Test ===");
        
        testPrimitiveArrays();
        testObjectArrays();
        testMultiDimensionalArrays();
        testArrayOperations();
        testArrayBounds();
        
        System.out.println("=== All Array Tests Completed ===");
    }
    
    static void testPrimitiveArrays() {
        System.out.println("\n--- Primitive Arrays Test ---");
        
        int[] intArray = new int[5];
        intArray[0] = 10;
        intArray[1] = 20;
        intArray[2] = 30;
        intArray[3] = 40;
        intArray[4] = 50;
        
        System.out.println("Int array length: " + intArray.length);
        System.out.println("Int array elements:");
        for (int i = 0; i < intArray.length; i++) {
            System.out.println("  intArray[" + i + "] = " + intArray[i]);
        }
        
        if (intArray.length == 5 && intArray[2] == 30) {
            System.out.println("Primitive int array: PASSED");
        } else {
            System.out.println("Primitive int array: FAILED");
        }
        
        double[] doubleArray = {1.1, 2.2, 3.3, 4.4, 5.5};
        System.out.println("Double array length: " + doubleArray.length);
        System.out.println("Double array elements:");
        for (int i = 0; i < doubleArray.length; i++) {
            System.out.println("  doubleArray[" + i + "] = " + doubleArray[i]);
        }
        
        if (doubleArray.length == 5 && doubleArray[3] == 4.4) {
            System.out.println("Primitive double array: PASSED");
        } else {
            System.out.println("Primitive double array: FAILED");
        }
        
        boolean[] boolArray = new boolean[3];
        boolArray[0] = true;
        boolArray[1] = false;
        boolArray[2] = true;
        
        System.out.println("Boolean array elements:");
        for (int i = 0; i < boolArray.length; i++) {
            System.out.println("  boolArray[" + i + "] = " + boolArray[i]);
        }
        
        if (boolArray[0] && !boolArray[1] && boolArray[2]) {
            System.out.println("Primitive boolean array: PASSED");
        } else {
            System.out.println("Primitive boolean array: FAILED");
        }
        
        char[] charArray = {'J', '2', 'M', 'E'};
        System.out.println("Char array: " + new String(charArray));
        
        if (charArray.length == 4 && charArray[1] == '2') {
            System.out.println("Primitive char array: PASSED");
        } else {
            System.out.println("Primitive char array: FAILED");
        }
    }
    
    static void testObjectArrays() {
        System.out.println("\n--- Object Arrays Test ---");
        
        String[] stringArray = new String[4];
        stringArray[0] = "Hello";
        stringArray[1] = "J2ME";
        stringArray[2] = "World";
        stringArray[3] = "Test";
        
        System.out.println("String array length: " + stringArray.length);
        System.out.println("String array elements:");
        for (int i = 0; i < stringArray.length; i++) {
            System.out.println("  stringArray[" + i + "] = " + stringArray[i]);
        }
        
        if (stringArray.length == 4 && stringArray[1].equals("J2ME")) {
            System.out.println("Object String array: PASSED");
        } else {
            System.out.println("Object String array: FAILED");
        }
        
        Integer[] integerArray = {new Integer(1), new Integer(2), new Integer(3)};
        System.out.println("Integer array elements:");
        for (int i = 0; i < integerArray.length; i++) {
            System.out.println("  integerArray[" + i + "] = " + integerArray[i].intValue());
        }
        
        if (integerArray.length == 3 && integerArray[1].intValue() == 2) {
            System.out.println("Object Integer array: PASSED");
        } else {
            System.out.println("Object Integer array: FAILED");
        }
    }
    
    static void testMultiDimensionalArrays() {
        System.out.println("\n--- Multi-dimensional Arrays Test ---");
        
        int[][] matrix = new int[3][3];
        matrix[0][0] = 1;
        matrix[0][1] = 2;
        matrix[0][2] = 3;
        matrix[1][0] = 4;
        matrix[1][1] = 5;
        matrix[1][2] = 6;
        matrix[2][0] = 7;
        matrix[2][1] = 8;
        matrix[2][2] = 9;
        
        System.out.println("Matrix dimensions: " + matrix.length + "x" + matrix[0].length);
        System.out.println("Matrix elements:");
        for (int i = 0; i < matrix.length; i++) {
            for (int j = 0; j < matrix[i].length; j++) {
                System.out.print(" " + matrix[i][j]);
            }
            System.out.println();
        }
        
        if (matrix.length == 3 && matrix[1][1] == 5) {
            System.out.println("2D int array: PASSED");
        } else {
            System.out.println("2D int array: FAILED");
        }
        
        int[][][] cube = new int[2][2][2];
        cube[0][0][0] = 1;
        cube[0][0][1] = 2;
        cube[0][1][0] = 3;
        cube[0][1][1] = 4;
        cube[1][0][0] = 5;
        cube[1][0][1] = 6;
        cube[1][1][0] = 7;
        cube[1][1][1] = 8;
        
        System.out.println("Cube dimensions: " + cube.length + "x" + cube[0].length + "x" + cube[0][0].length);
        System.out.println("Cube element [1][1][1]: " + cube[1][1][1]);
        
        if (cube.length == 2 && cube[1][1][1] == 8) {
            System.out.println("3D int array: PASSED");
        } else {
            System.out.println("3D int array: FAILED");
        }
    }
    
    static void testArrayOperations() {
        System.out.println("\n--- Array Operations Test ---");
        
        int[] array1 = {1, 2, 3, 4, 5};
        int[] array2 = new int[array1.length];
        
        System.out.println("Copying array1 to array2");
        for (int i = 0; i < array1.length; i++) {
            array2[i] = array1[i];
        }
        
        boolean copyCorrect = true;
        for (int i = 0; i < array1.length; i++) {
            if (array1[i] != array2[i]) {
                copyCorrect = false;
                break;
            }
        }
        
        if (copyCorrect) {
            System.out.println("Array copy: PASSED");
        } else {
            System.out.println("Array copy: FAILED");
        }
        
        int sum = 0;
        for (int i = 0; i < array1.length; i++) {
            sum += array1[i];
        }
        System.out.println("Sum of array elements: " + sum);
        
        if (sum == 15) {
            System.out.println("Array sum: PASSED");
        } else {
            System.out.println("Array sum: FAILED");
        }
        
        int max = array1[0];
        int min = array1[0];
        for (int i = 1; i < array1.length; i++) {
            if (array1[i] > max) max = array1[i];
            if (array1[i] < min) min = array1[i];
        }
        System.out.println("Max: " + max + ", Min: " + min);
        
        if (max == 5 && min == 1) {
            System.out.println("Array max/min: PASSED");
        } else {
            System.out.println("Array max/min: FAILED");
        }
        
        int[] reversed = new int[array1.length];
        for (int i = 0; i < array1.length; i++) {
            reversed[i] = array1[array1.length - 1 - i];
        }
        
        System.out.println("Reversed array:");
        for (int i = 0; i < reversed.length; i++) {
            System.out.print(" " + reversed[i]);
        }
        System.out.println();
        
        if (reversed[0] == 5 && reversed[4] == 1) {
            System.out.println("Array reverse: PASSED");
        } else {
            System.out.println("Array reverse: FAILED");
        }
    }
    
    static void testArrayBounds() {
        System.out.println("\n--- Array Bounds Test ---");
        
        int[] array = new int[5];
        array[0] = 1;
        array[4] = 5;
        
        System.out.println("Accessing valid indices:");
        System.out.println("array[0] = " + array[0]);
        System.out.println("array[4] = " + array[4]);
        
        if (array[0] == 1 && array[4] == 5) {
            System.out.println("Valid array access: PASSED");
        } else {
            System.out.println("Valid array access: FAILED");
        }
        
        System.out.println("Testing array length property:");
        System.out.println("array.length = " + array.length);
        
        if (array.length == 5) {
            System.out.println("Array length: PASSED");
        } else {
            System.out.println("Array length: FAILED");
        }
        
        System.out.println("Testing empty array:");
        int[] emptyArray = new int[0];
        System.out.println("emptyArray.length = " + emptyArray.length);
        
        if (emptyArray.length == 0) {
            System.out.println("Empty array: PASSED");
        } else {
            System.out.println("Empty array: FAILED");
        }
    }
}
