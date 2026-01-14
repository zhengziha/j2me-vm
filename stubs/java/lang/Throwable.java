package java.lang;

public class Throwable {
    private String detailMessage;
    private Throwable cause;

    public Throwable() {
        this.detailMessage = null;
        this.cause = this;
    }

    public Throwable(String message) {
        this.detailMessage = message;
        this.cause = this;
    }

    public Throwable(String message, Throwable cause) {
        this.detailMessage = message;
        this.cause = cause;
    }

    public Throwable(Throwable cause) {
        this.detailMessage = (cause == null ? null : cause.toString());
        this.cause = cause;
    }

    public String getMessage() {
        return detailMessage;
    }

    public String getLocalizedMessage() {
        return getMessage();
    }

    public Throwable getCause() {
        return (cause == this ? null : cause);
    }

    public Throwable initCause(Throwable cause) {
        if (this.cause != this) {
            throw new IllegalStateException("Can't overwrite cause");
        }
        if (cause == this) {
            throw new IllegalArgumentException("Self-causation not permitted");
        }
        this.cause = cause;
        return this;
    }

    public String toString() {
        String s = getClass().getName();
        String message = getMessage();
        return (message != null) ? (s + ": " + message) : s;
    }

    public void printStackTrace() {
        System.out.println(toString());
    }

    public StackTraceElement[] getStackTrace() {
        return new StackTraceElement[0];
    }

    public void setStackTrace(StackTraceElement[] stackTrace) {
    }
}
