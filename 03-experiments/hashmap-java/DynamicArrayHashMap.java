/**
 * Chaining-based HashMap using hand-rolled DynamicArray
 */
public class DynamicArrayHashMap<K, V> {
    private static final int DEFAULT_CAPACITY = 16;
    private static final double LOAD_FACTOR = 0.75;

    private DynamicArray<Entry<K, V>>[] buckets;
    private int size;

    @SuppressWarnings("unchecked")
    public DynamicArrayHashMap() {
        buckets = new DynamicArray[DEFAULT_CAPACITY];
        for (int i = 0; i < DEFAULT_CAPACITY; i++) {
            buckets[i] = new DynamicArray<>();
        }
        size = 0;
    }

    private int getBucketIndex(K key) {
        if (key == null) {
            throw new IllegalArgumentException("null keys not supported");
        }
        return Math.floorMod(key.hashCode(), buckets.length);
    }

    public void put(K key, V value) {
        int index = getBucketIndex(key);
        DynamicArray<Entry<K, V>> bucket = buckets[index];

        for (int i = 0; i < bucket.size(); i++) {
            Entry<K, V> entry = bucket.get(i);
            if (entry.key.equals(key)) {
                entry.value = value;
                return;
            }
        }

        bucket.add(new Entry<>(key, value));
        size++;

        if ((double) size / buckets.length > LOAD_FACTOR) {
            resize();
        }
    }

    public V get(K key) {
        int index = getBucketIndex(key);
        DynamicArray<Entry<K, V>> bucket = buckets[index];

        for (int i = 0; i < bucket.size(); i++) {
            Entry<K, V> entry = bucket.get(i);
            if (entry.key.equals(key)) {
                return entry.value;
            }
        }
        return null;
    }

    public V remove(K key) {
        int index = getBucketIndex(key);
        DynamicArray<Entry<K, V>> bucket = buckets[index];

        for (int i = 0; i < bucket.size(); i++) {
            Entry<K, V> entry = bucket.get(i);
            if (entry.key.equals(key)) {
                bucket.remove(i);
                size--;
                return entry.value;
            }
        }
        return null;
    }

    public boolean containsKey(K key) {
        int index = getBucketIndex(key);
        DynamicArray<Entry<K, V>> bucket = buckets[index];
        for (int i = 0; i < bucket.size(); i++) {
            if (bucket.get(i).key.equals(key)) return true;
        }
        return false;
    }

    public int size() {
        return size;
    }

    @SuppressWarnings("unchecked")
    private void resize() {
        DynamicArray<Entry<K, V>>[] oldBuckets = buckets;
        buckets = new DynamicArray[oldBuckets.length * 2];

        for (int i = 0; i < buckets.length; i++) {
            buckets[i] = new DynamicArray<>();
        }

        size = 0;

        // Rehash required: bucket index depends on array length
        for (DynamicArray<Entry<K, V>> bucket : oldBuckets) {
            for (int i = 0; i < bucket.size(); i++) {
                Entry<K, V> entry = bucket.get(i);
                put(entry.key, entry.value);
            }
        }
    }

    public void printBuckets() {
        System.out.println("=== HashMap (size=" + size + ", buckets=" + buckets.length + ") ===");
        for (int i = 0; i < buckets.length; i++) {
            if (!buckets[i].isEmpty()) {
                System.out.print("Bucket " + i + ": ");
                for (int j = 0; j < buckets[i].size(); j++) {
                    Entry<K, V> entry = buckets[i].get(j);
                    System.out.print("(" + entry.key + "," + entry.value + ") ");
                }
                System.out.println();
            }
        }
    }
}
