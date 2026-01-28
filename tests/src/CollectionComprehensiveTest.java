import java.util.Vector;
import java.util.Hashtable;
import java.util.Stack;
import java.util.Enumeration;

public class CollectionComprehensiveTest {
    private static int passed = 0;
    private static int failed = 0;

    public static int getPassed() {
        return passed;
    }

    public static int getFailed() {
        return failed;
    }

    public static void assertTrue(String description, boolean condition) {
        if (condition) {
            System.out.println("  [PASS] " + description);
            passed++;
        } else {
            System.out.println("  [FAIL] " + description);
            failed++;
        }
    }

    public static void assertEquals(String description, int expected, int actual) {
        assertTrue(description, expected == actual);
    }

    public static void assertEquals(String description, String expected, String actual) {
        assertTrue(description, (expected == null && actual == null) || 
                            (expected != null && expected.equals(actual)));
    }

    public static void assertEquals(String description, Boolean expected, Boolean actual) {
        assertTrue(description, expected.equals(actual));
    }

    public static void testVector() {
        System.out.println("\n--- Vector Tests ---");

        Vector v = new Vector();
        assertEquals("Empty vector size", 0, v.size());
        assertTrue("Empty vector isEmpty", v.isEmpty());

        v.addElement("A");
        v.addElement("B");
        v.addElement("C");
        assertEquals("Add elements size", 3, v.size());
        assertTrue("!isEmpty after add", !v.isEmpty());

        assertTrue("Contains A", v.contains("A"));
        assertTrue("!Contains X", !v.contains("X"));

        assertEquals("elementAt(0)", "A", (String)v.elementAt(0));
        assertEquals("elementAt(1)", "B", (String)v.elementAt(1));
        assertEquals("elementAt(2)", "C", (String)v.elementAt(2));

        assertEquals("indexOf(A)", 0, v.indexOf("A"));
        assertEquals("indexOf(C)", 2, v.indexOf("C"));
        assertEquals("indexOf(X)", -1, v.indexOf("X"));

        v.insertElementAt("D", 1);
        assertEquals("Insert size", 4, v.size());
        assertEquals("Insert position", "D", (String)v.elementAt(1));

        v.removeElement("B");
        assertEquals("Remove size", 3, v.size());
        assertTrue("!Contains after remove", !v.contains("B"));

        v.setElementAt("E", 0);
        assertEquals("Set element", "E", (String)v.elementAt(0));

        v.removeElementAt(2);
        assertEquals("Remove at size", 2, v.size());

        Object[] arr = new Object[2];
        v.copyInto(arr);
        assertEquals("copyInto[0]", "E", (String)arr[0]);
        assertEquals("copyInto[1]", "D", (String)arr[1]);

        v.removeAllElements();
        assertEquals("Clear size", 0, v.size());
        assertTrue("isEmpty after clear", v.isEmpty());
    }

    public static void testVectorEnumeration() {
        System.out.println("\n--- Vector Enumeration ---");

        Vector v = new Vector();
        v.addElement("A");
        v.addElement("B");
        v.addElement("C");

        Enumeration e = v.elements();
        assertTrue("hasMoreElements", e.hasMoreElements());
        assertEquals("firstElement", "A", (String)e.nextElement());
        assertEquals("secondElement", "B", (String)e.nextElement());
        assertEquals("thirdElement", "C", (String)e.nextElement());
        assertTrue("!hasMoreElements after all", !e.hasMoreElements());
    }

    public static void testHashtable() {
        System.out.println("\n--- Hashtable Tests ---");

        Hashtable ht = new Hashtable();
        assertEquals("Empty hashtable size", 0, ht.size());
        assertTrue("Empty hashtable isEmpty", ht.isEmpty());

        ht.put("key1", "value1");
        ht.put("key2", "value2");
        ht.put("key3", "value3");
        assertEquals("Put size", 3, ht.size());
        assertTrue("!isEmpty after put", !ht.isEmpty());

        assertEquals("Get key1", "value1", (String)ht.get("key1"));
        assertEquals("Get key2", "value2", (String)ht.get("key2"));
        assertEquals("Get key3", "value3", (String)ht.get("key3"));
        assertTrue("Get missing key", ht.get("key4") == null);

        assertTrue("Contains key key1", ht.containsKey("key1"));
        assertTrue("Contains key key2", ht.containsKey("key2"));
        assertTrue("!Contains key missing", !ht.containsKey("missing"));

        assertTrue("Contains value value1", ht.contains("value1"));
        assertTrue("Contains value value2", ht.contains("value2"));
        assertTrue("!Contains value missing", !ht.contains("missing"));

        ht.put("key1", "newvalue1");
        assertEquals("Replace value", "newvalue1", (String)ht.get("key1"));

        Object removed = ht.remove("key2");
        assertEquals("Remove returns", "value2", (String)removed);
        assertEquals("Remove size", 2, ht.size());
        assertTrue("!Contains key after remove", !ht.containsKey("key2"));

        ht.clear();
        assertEquals("Clear size", 0, ht.size());
        assertTrue("isEmpty after clear", ht.isEmpty());
    }

    public static void testHashtableEnumeration() {
        System.out.println("\n--- Hashtable Enumeration ---");

        Hashtable ht = new Hashtable();
        ht.put("key1", "value1");
        ht.put("key2", "value2");
        ht.put("key3", "value3");

        Enumeration keys = ht.keys();
        int keyCount = 0;
        while (keys.hasMoreElements()) {
            keys.nextElement();
            keyCount++;
        }
        assertEquals("Keys count", 3, keyCount);

        Enumeration values = ht.elements();
        int valueCount = 0;
        while (values.hasMoreElements()) {
            values.nextElement();
            valueCount++;
        }
        assertEquals("Values count", 3, valueCount);
    }

    public static void testStack() {
        System.out.println("\n--- Stack Tests ---");

        Stack s = new Stack();
        assertEquals("Empty stack size", 0, s.size());
        assertTrue("Empty stack isEmpty", s.isEmpty());

        s.push("A");
        s.push("B");
        s.push("C");
        assertEquals("Push size", 3, s.size());

        assertEquals("Peek", "C", (String)s.peek());
        assertEquals("Peek doesn't remove", 3, s.size());

        assertEquals("Pop", "C", (String)s.pop());
        assertEquals("Pop reduces size", 2, s.size());
        assertEquals("Next peek", "B", (String)s.peek());

        assertEquals("Pop B", "B", (String)s.pop());
        assertEquals("Pop A", "A", (String)s.pop());
        assertEquals("Pop all size", 0, s.size());
        assertTrue("isEmpty after pop all", s.isEmpty());

        try {
            s.pop();
            assertTrue("Pop empty should throw", false);
        } catch (Exception e) {
            assertTrue("Pop empty throws exception", true);
        }
    }

    public static void testStackSearch() {
        System.out.println("\n--- Stack Search ---");

        Stack s = new Stack();
        s.push("A");
        s.push("B");
        s.push("C");

        assertEquals("Search C", 1, s.search("C"));
        assertEquals("Search B", 2, s.search("B"));
        assertEquals("Search A", 3, s.search("A"));
        assertEquals("Search missing", -1, s.search("X"));
    }

    public static void testVectorCapacity() {
        System.out.println("\n--- Vector Capacity ---");

        Vector v = new Vector(10);
        assertTrue("Initial capacity", v.capacity() >= 10);

        v.ensureCapacity(20);
        assertTrue("ensureCapacity", v.capacity() >= 20);

        v.setSize(5);
        assertEquals("Set size", 5, v.size());

        v.trimToSize();
        assertTrue("trimToSize", v.capacity() >= v.size());
    }

    public static void testMixedTypes() {
        System.out.println("\n--- Mixed Types ---");

        Vector v = new Vector();
        v.addElement("String");
        v.addElement(new Integer(42));
        v.addElement(new Boolean(true));

        assertEquals("String element", "String", (String)v.elementAt(0));
        assertEquals("Integer element", new Integer(42), (Integer)v.elementAt(1));
        assertEquals("Boolean element", new Boolean(true), (Boolean)v.elementAt(2));

        Hashtable ht = new Hashtable();
        ht.put(1, "one");
        ht.put("two", 2);
        ht.put(new Boolean(true), new Integer(3));

        assertEquals("Integer key", "one", (String)ht.get(new Integer(1)));
        assertEquals("String key", new Integer(2), (Integer)ht.get("two"));
        assertEquals("Boolean key", new Integer(3), (Integer)ht.get(new Boolean(true)));
    }

    public static void main(String[] args) {
        System.out.println("=== Collection Comprehensive Test ===");

        testVector();
        testVectorEnumeration();
        testHashtable();
        testHashtableEnumeration();
        testStack();
        testStackSearch();
        testVectorCapacity();
        testMixedTypes();

        System.out.println("\n=== Test Summary ===");
        System.out.println("Passed: " + passed);
        System.out.println("Failed: " + failed);
        System.out.println("Total: " + (passed + failed));

        if (failed == 0) {
            System.out.println("ALL TESTS PASSED!");
        } else {
            System.out.println("SOME TESTS FAILED!");
        }
    }
}
