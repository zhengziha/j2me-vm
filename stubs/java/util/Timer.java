package java.util;

public class Timer {
    public Timer() {
    }

    public void schedule(TimerTask task, long delay) {
        if (delay < 0) throw new IllegalArgumentException("Negative delay.");
        sched(task, delay, 0);
    }

    public void schedule(TimerTask task, Date time) {
        sched(task, time.getTime() - System.currentTimeMillis(), 0);
    }

    public void schedule(TimerTask task, long delay, long period) {
        if (delay < 0) throw new IllegalArgumentException("Negative delay.");
        if (period <= 0) throw new IllegalArgumentException("Non-positive period.");
        sched(task, delay, -period);
    }

    public void schedule(TimerTask task, Date firstTime, long period) {
        if (period <= 0) throw new IllegalArgumentException("Non-positive period.");
        sched(task, firstTime.getTime() - System.currentTimeMillis(), -period);
    }

    public void scheduleAtFixedRate(TimerTask task, long delay, long period) {
        if (delay < 0) throw new IllegalArgumentException("Negative delay.");
        if (period <= 0) throw new IllegalArgumentException("Non-positive period.");
        sched(task, delay, period);
    }

    public void scheduleAtFixedRate(TimerTask task, Date firstTime, long period) {
        if (period <= 0) throw new IllegalArgumentException("Non-positive period.");
        sched(task, firstTime.getTime() - System.currentTimeMillis(), period);
    }

    private void sched(TimerTask task, long delay, long period) {
        if (delay < 0) delay = 0;
        scheduleNative(task, delay, period);
    }

    private native void scheduleNative(TimerTask task, long delay, long period);

    public void cancel() {
        // TODO: Cancel all tasks
    }
}
