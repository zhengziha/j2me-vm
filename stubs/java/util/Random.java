package java.util;

public class Random {
    private long seed;
    private static final long multiplier = 0x5DEECE66DL;
    private static final long addend = 0xB;
    private static final long mask = (1L << 48) - 1;

    public Random() {
        this(System.currentTimeMillis());
    }

    public Random(long seed) {
        setSeed(seed);
    }

    public synchronized void setSeed(long seed) {
        this.seed = (seed ^ multiplier) & mask;
    }

    protected synchronized int next(int bits) {
        long nextseed = (seed * multiplier + addend) & mask;
        seed = nextseed;
        return (int)(nextseed >>> (48 - bits));
    }

    public int nextInt() {
        return next(32);
    }

    public int nextInt(int n) {
        if (n <= 0) {
            throw new IllegalArgumentException("n must be positive");
        }
        if ((n & -n) == n) {
            return (int)((n * (long)next(31)) >> 31);
        }
        int bits, val;
        do {
            bits = next(31);
            val = bits % n;
        } while (bits - val + (n - 1) < 0);
        return val;
    }

    public long nextLong() {
        return ((long)next(32) << 32) + next(32);
    }
    
    public float nextFloat() {
        return next(24) / ((float)(1 << 24));
    }
    
    public double nextDouble() {
         return (((long)next(26) << 27) + next(27)) / (double)(1L << 53);
    }
}
