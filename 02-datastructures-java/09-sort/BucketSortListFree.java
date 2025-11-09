import java.util.Arrays;

public class BucketSortListFree {
    private static void insertionSortRange(int[] a, int lo, int hi) {
        for (int i = lo + 1; i < hi; i++) {
            int key = a[i];
            int j = i - 1;
            while (j >= lo && a[j] > key) {
                a[j + 1] = a[j];
                j--;
            }
            a[j + 1] = key;
        }
    }

    private static int[] minMax(int[] a) {
        int mn = a[0];
        int mx = a[0];
        for (int i = 1; i < a.length; i++) {
            int v = a[i];
            if (v < mn) mn = v;
            if (v > mx) mx = v;
        }
        return new int[]{mn, mx};
    }

    private static int bucketIndex(int x, int min, long range, int B) {
        // Map [min, max] into [0, B) using scaled integer division.
        long num = ((long) x - (long) min) * (long) B;  // 0 .. B*range-1 ideally
        int b = (int) (num / range);                    // floor
        // Clamp for safety against rounding / overflow corner cases.
        if (b < 0) return 0;
        if (b >= B) return B - 1;
        return b;
    }

    public static void bucketSortListFreeBuffered(int[] a) {
        final int n = a.length;
        if (n <= 1) return;

        final int B = n;

        int[] mm = minMax(a);
        int min = mm[0];
        int max = mm[1];

        // All values equal → already sorted.
        if (min == max) return;

        final long range = (long) max - (long) min + 1L;

        // 1) Count how many elements go into each bucket.
        int[] cnt = new int[B];
        for (int x : a) {
            int b = bucketIndex(x, min, range, B);
            cnt[b]++;
        }

        // 2) Prefix sums → starting index of each bucket's slice in A[0..n-1].
        int[] start = new int[B];
        for (int b = 1; b < B; b++) {
            start[b] = start[b - 1] + cnt[b - 1];
        }

        // next[b] is the next free position inside bucket b.
        int[] next = start.clone();

        // 3) Counting-sort style scatter into a temporary buffer T.
        int[] T = new int[n];
        for (int x : a) {
            int b = bucketIndex(x, min, range, B);
            T[next[b]++] = x;
        }

        // Overwrite A with the bucket-partitioned buffer T,
        // so that each bucket b occupies A[start[b] .. start[b]+cnt[b]) contiguously.
        System.arraycopy(T, 0, a, 0, n);

        // 4) Local sort inside each bucket slice using insertion sort.
        for (int b = 0; b < B; b++) {
            int lo = start[b];
            int hi = lo + cnt[b];      // slice is [lo, hi)
            insertionSortRange(a, lo, hi);
        }
    }

    /* ---------- demo ---------- */
    public static void main(String[] args) {
        int[] a = {29, 25, 3, 49, 9, 37, 21, 43};

        System.out.println("Before: " + Arrays.toString(a));
        bucketSortListFreeBuffered(a);
        System.out.println("After:  " + Arrays.toString(a));
    }
}