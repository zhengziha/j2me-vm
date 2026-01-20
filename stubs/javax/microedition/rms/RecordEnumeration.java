package javax.microedition.rms;

public interface RecordEnumeration {
    void destroy();
    boolean hasNextElement();
    boolean hasPreviousElement();
    boolean isKeptUpdated();
    void keepUpdated(boolean keepUpdated);
    byte[] nextRecord() throws RecordStoreNotOpenException, RecordStoreException, InvalidRecordIDException;
    int nextRecordId() throws InvalidRecordIDException;
    int numRecords();
    byte[] previousRecord() throws RecordStoreNotOpenException, RecordStoreException, InvalidRecordIDException;
    int previousRecordId() throws InvalidRecordIDException;
    void rebuild();
    void reset();
}
