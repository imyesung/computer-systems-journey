public class HashMapTest {
    public static void main(String[] args) {
        System.out.println("===== ArrayList-based HashMap =====\n");
        testArrayListHashMap();

        System.out.println("\n===== DynamicArray-based HashMap =====\n");
        testDynamicArrayHashMap();
    }

    static void testArrayListHashMap() {
        ArrayListHashMap<String, Integer> map = new ArrayListHashMap<>();

        System.out.println("--- put ---");
        map.put("apple", 100);
        map.put("banana", 200);
        map.put("cherry", 300);
        map.printBuckets();

        System.out.println("\n--- get ---");
        System.out.println("apple: " + map.get("apple"));
        System.out.println("banana: " + map.get("banana"));
        System.out.println("missing: " + map.get("missing"));

        System.out.println("\n--- update ---");
        map.put("apple", 999);
        System.out.println("apple: " + map.get("apple"));

        System.out.println("\n--- remove ---");
        System.out.println("removed: " + map.remove("banana"));
        System.out.println("banana: " + map.get("banana"));
        System.out.println("size: " + map.size());

        System.out.println("\n--- resize trigger ---");
        for (int i = 0; i < 20; i++) {
            map.put("key" + i, i);
        }
        System.out.println("size: " + map.size());
        map.printBuckets();

        System.out.println("\n--- edge cases ---");
        // Integer.MIN_VALUE hashCode test
        String edgeKey = "Polygenelubricants";
        System.out.println("hashCode of edgeKey: " + edgeKey.hashCode());
        map.put(edgeKey, 777);
        System.out.println("get(edgeKey): " + map.get(edgeKey));

        // null value test
        map.put("nullVal", null);
        System.out.println("containsKey(nullVal): " + map.containsKey("nullVal"));
        System.out.println("get(nullVal): " + map.get("nullVal"));
    }

    static void testDynamicArrayHashMap() {
        DynamicArrayHashMap<String, Integer> map = new DynamicArrayHashMap<>();

        System.out.println("--- put ---");
        map.put("apple", 100);
        map.put("banana", 200);
        map.put("cherry", 300);
        map.printBuckets();

        System.out.println("\n--- get ---");
        System.out.println("apple: " + map.get("apple"));
        System.out.println("banana: " + map.get("banana"));
        System.out.println("missing: " + map.get("missing"));

        System.out.println("\n--- update ---");
        map.put("apple", 999);
        System.out.println("apple: " + map.get("apple"));

        System.out.println("\n--- remove ---");
        System.out.println("removed: " + map.remove("banana"));
        System.out.println("banana: " + map.get("banana"));
        System.out.println("size: " + map.size());

        System.out.println("\n--- resize trigger ---");
        for (int i = 0; i < 20; i++) {
            map.put("key" + i, i);
        }
        System.out.println("size: " + map.size());
        map.printBuckets();
    }
}
