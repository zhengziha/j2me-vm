import java.util.Vector;
import java.util.Hashtable;
import java.util.Stack;
import java.util.Enumeration;

public class CollectionTest {
    public static void main(String[] args) {
        System.out.println("=== Collection Test ===");
        
        testVector();
        testHashtable();
        testStack();
        testEnumeration();
        
        System.out.println("=== All Collection Tests Completed ===");
    }
    
    static void testVector() {
        System.out.println("\n--- Vector Test ---");
        
        Vector vector = new Vector();
        
        System.out.println("Adding elements to vector...");
        vector.addElement("Apple");
        vector.addElement("Banana");
        vector.addElement("Cherry");
        vector.addElement(new Integer(42));
        vector.addElement(new Double(3.14));
        
        System.out.println("Vector size: " + vector.size());
        System.out.println("Vector capacity: " + vector.capacity());
        
        if (vector.size() == 5) {
            System.out.println("Vector size: PASSED");
        } else {
            System.out.println("Vector size: FAILED");
        }
        
        System.out.println("Vector elements:");
        for (int i = 0; i < vector.size(); i++) {
            System.out.println("  [" + i + "] = " + vector.elementAt(i));
        }
        
        Object firstElement = vector.firstElement();
        Object lastElement = vector.lastElement();
        
        System.out.println("First element: " + firstElement);
        System.out.println("Last element: " + lastElement);
        
        if (firstElement.equals("Apple") && lastElement.toString().equals("3.14")) {
            System.out.println("Vector first/last element: PASSED");
        } else {
            System.out.println("Vector first/last element: FAILED");
        }
        
        boolean contains = vector.contains("Banana");
        System.out.println("Contains 'Banana': " + contains);
        
        if (contains) {
            System.out.println("Vector contains: PASSED");
        } else {
            System.out.println("Vector contains: FAILED");
        }
        
        int index = vector.indexOf("Cherry");
        System.out.println("Index of 'Cherry': " + index);
        
        if (index == 2) {
            System.out.println("Vector indexOf: PASSED");
        } else {
            System.out.println("Vector indexOf: FAILED");
        }
        
        vector.removeElementAt(1);
        System.out.println("After removing element at index 1, size: " + vector.size());
        
        if (vector.size() == 4) {
            System.out.println("Vector remove: PASSED");
        } else {
            System.out.println("Vector remove: FAILED");
        }
        
        vector.insertElementAt("Orange", 1);
        System.out.println("After inserting 'Orange' at index 1: " + vector.elementAt(1));
        
        if (vector.elementAt(1).equals("Orange")) {
            System.out.println("Vector insert: PASSED");
        } else {
            System.out.println("Vector insert: FAILED");
        }
        
        vector.setElementAt("Grape", 1);
        System.out.println("After setting element at index 1 to 'Grape': " + vector.elementAt(1));
        
        if (vector.elementAt(1).equals("Grape")) {
            System.out.println("Vector set: PASSED");
        } else {
            System.out.println("Vector set: FAILED");
        }
    }
    
    static void testHashtable() {
        System.out.println("\n--- Hashtable Test ---");
        
        Hashtable hashtable = new Hashtable();
        
        System.out.println("Adding key-value pairs to hashtable...");
        hashtable.put("name", "J2ME");
        hashtable.put("version", "2.0");
        hashtable.put("platform", "Mobile");
        hashtable.put(new Integer(1), "One");
        hashtable.put(new Integer(2), "Two");
        
        System.out.println("Hashtable size: " + hashtable.size());
        
        if (hashtable.size() == 5) {
            System.out.println("Hashtable size: PASSED");
        } else {
            System.out.println("Hashtable size: FAILED");
        }
        
        String name = (String) hashtable.get("name");
        System.out.println("Value for key 'name': " + name);
        
        if (name.equals("J2ME")) {
            System.out.println("Hashtable get: PASSED");
        } else {
            System.out.println("Hashtable get: FAILED");
        }
        
        boolean containsKey = hashtable.containsKey("version");
        boolean containsValue = hashtable.contains("Mobile");
        
        System.out.println("Contains key 'version': " + containsKey);
        System.out.println("Contains value 'Mobile': " + containsValue);
        
        if (containsKey && containsValue) {
            System.out.println("Hashtable contains: PASSED");
        } else {
            System.out.println("Hashtable contains: FAILED");
        }
        
        Object removed = hashtable.remove("platform");
        System.out.println("Removed value: " + removed);
        System.out.println("Hashtable size after removal: " + hashtable.size());
        
        if (hashtable.size() == 4) {
            System.out.println("Hashtable remove: PASSED");
        } else {
            System.out.println("Hashtable remove: FAILED");
        }
        
        System.out.println("Hashtable keys:");
        Enumeration keys = hashtable.keys();
        while (keys.hasMoreElements()) {
            System.out.println("  " + keys.nextElement());
        }
        
        System.out.println("Hashtable values:");
        Enumeration values = hashtable.elements();
        while (values.hasMoreElements()) {
            System.out.println("  " + values.nextElement());
        }
        
        hashtable.clear();
        System.out.println("Hashtable size after clear: " + hashtable.size());
        
        if (hashtable.size() == 0) {
            System.out.println("Hashtable clear: PASSED");
        } else {
            System.out.println("Hashtable clear: FAILED");
        }
    }
    
    static void testStack() {
        System.out.println("\n--- Stack Test ---");
        
        Stack stack = new Stack();
        
        System.out.println("Pushing elements onto stack...");
        stack.push("First");
        stack.push("Second");
        stack.push("Third");
        stack.push(new Integer(42));
        
        System.out.println("Stack size: " + stack.size());
        
        if (stack.size() == 4) {
            System.out.println("Stack size: PASSED");
        } else {
            System.out.println("Stack size: FAILED");
        }
        
        Object top = stack.peek();
        System.out.println("Top element (peek): " + top);
        
        if (top.equals(new Integer(42))) {
            System.out.println("Stack peek: PASSED");
        } else {
            System.out.println("Stack peek: FAILED");
        }
        
        Object popped = stack.pop();
        System.out.println("Popped element: " + popped);
        System.out.println("Stack size after pop: " + stack.size());
        
        if (stack.size() == 3) {
            System.out.println("Stack pop: PASSED");
        } else {
            System.out.println("Stack pop: FAILED");
        }
        
        int position = stack.search("Second");
        System.out.println("Position of 'Second' from top: " + position);
        
        if (position == 2) {
            System.out.println("Stack search: PASSED");
        } else {
            System.out.println("Stack search: FAILED");
        }
        
        boolean empty = stack.empty();
        System.out.println("Stack is empty: " + empty);
        
        if (!empty) {
            System.out.println("Stack empty: PASSED");
        } else {
            System.out.println("Stack empty: FAILED");
        }
        
        System.out.println("Popping all elements:");
        while (!stack.empty()) {
            System.out.println("  Popped: " + stack.pop());
        }
        
        if (stack.empty()) {
            System.out.println("Stack empty after popping all: PASSED");
        } else {
            System.out.println("Stack empty after popping all: FAILED");
        }
    }
    
    static void testEnumeration() {
        System.out.println("\n--- Enumeration Test ---");
        
        Vector vector = new Vector();
        vector.addElement("Red");
        vector.addElement("Green");
        vector.addElement("Blue");
        vector.addElement("Yellow");
        
        System.out.println("Using Enumeration to iterate Vector:");
        Enumeration e = vector.elements();
        int count = 0;
        while (e.hasMoreElements()) {
            String color = (String) e.nextElement();
            System.out.println("  Color: " + color);
            count++;
        }
        
        if (count == 4) {
            System.out.println("Enumeration iteration: PASSED");
        } else {
            System.out.println("Enumeration iteration: FAILED");
        }
        
        Hashtable hashtable = new Hashtable();
        hashtable.put("1", "One");
        hashtable.put("2", "Two");
        hashtable.put("3", "Three");
        
        System.out.println("Using Enumeration to iterate Hashtable keys:");
        Enumeration keys = hashtable.keys();
        int keyCount = 0;
        while (keys.hasMoreElements()) {
            Object key = keys.nextElement();
            Object value = hashtable.get(key);
            System.out.println("  Key: " + key + ", Value: " + value);
            keyCount++;
        }
        
        if (keyCount == 3) {
            System.out.println("Hashtable key enumeration: PASSED");
        } else {
            System.out.println("Hashtable key enumeration: FAILED");
        }
    }
}
