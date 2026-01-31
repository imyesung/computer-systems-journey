/**
 * Hand-rolled dynamic array (same concept as ArrayList)
 */
public class DynamicArray<T> {
    private static final int DEFAULT_CAPACITY = 4;

    private Object[] data;
    private int size;

    public DynamicArray() {
        data = new Object[DEFAULT_CAPACITY];
        size = 0;
    }

    public void add(T element) {
        if (size == data.length) {
            resize();
        }
        data[size++] = element;
    }

    @SuppressWarnings("unchecked")
    public T get(int index) {
        if (index < 0 || index >= size) {
            throw new IndexOutOfBoundsException("Index: " + index + ", Size: " + size);
        }
        return (T) data[index];
    }

    @SuppressWarnings("unchecked")
    public T remove(int index) {
        if (index < 0 || index >= size) {
            throw new IndexOutOfBoundsException("Index: " + index + ", Size: " + size);
        }

        T removed = (T) data[index];

        // Shift elements left to fill the gap
        for (int i = index; i < size - 1; i++) {
            data[i] = data[i + 1];
        }
        data[--size] = null;

        return removed;
    }

    public int size() {
        return size;
    }

    public boolean isEmpty() {
        return size == 0;
    }

    private void resize() {
        Object[] newData = new Object[data.length * 2];
        for (int i = 0; i < size; i++) {
            newData[i] = data[i];
        }
        data = newData;
    }
}
