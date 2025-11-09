public class OptimizedMergeSort {

    private static void merge(int[] src, int[] dst,
                              int left, int mid, int right) {
        int i = left; 
        int j = mid + 1;
        int k = left;
        
        while (i <= mid && j <= right) {
            if (src[i] <= src[j]) {
                dst[k++] = src[i++];
            } else {
                dst[k++] = src[j++];
            }
        }

        while (i <= mid) {
            dst[k++] = src[i++];
        }

        while (j <= right) {
            dst[k++] = src[j++];
        }
    }

    /**
     * Recursively sorts by swapping src/dst roles.
     * After recursion, src contains sorted subarrays to merge into dst.
     */
    private static void sort(int[] src, int[] dst,
                             int left, int right) {
        if (left >= right) {
            dst[left] = src[left];
            return;
        }

        int mid = (left + right) / 2;

        // swap roles: dst becomes src, src becomes dst
        sort(dst, src, left,     mid);
        sort(dst, src, mid + 1,  right);

        merge(src, dst, left, mid, right);
    }

    /** Sorts the array in ascending order using optimized merge sort. */
    public static void mergeSort(int[] arr) {
        int n = arr.length;
        if (n <= 1) {
            return;
        }
        int[] tmp = new int[n];
        System.arraycopy(arr, 0, tmp, 0, n);

        sort(tmp, arr, 0, n - 1);
    }
    public static void main(String[] args) {
        int[] arr = {9, 23, 4, 719, 3, 77, 1, 2};

        System.out.print("Before: ");
        for (int x : arr) {
            System.out.print(x + " ");
        }
        System.out.println();

        mergeSort(arr);

        System.out.print("After:  ");
        for (int x : arr) {
            System.out.print(x + " ");
        }
        System.out.println();
    }
}