package javax.microedition.rms;

public class RecordStore {
    private String name;
    private int nativePtr;

    private RecordStore(String name, int ptr) {
        this.name = name;
        this.nativePtr = ptr;
    }

    public static RecordStore openRecordStore(String recordStoreName, boolean createIfNecessary) throws RecordStoreException {
        int ptr = openRecordStoreNative(recordStoreName, createIfNecessary);
        if (ptr == 0) {
            throw new RecordStoreException("Failed to open record store: " + recordStoreName);
        }
        return new RecordStore(recordStoreName, ptr);
    }

    public int addRecord(byte[] data, int offset, int numBytes) throws RecordStoreNotOpenException, RecordStoreFullException, RecordStoreException {
        if (nativePtr == 0) {
            throw new RecordStoreNotOpenException();
        }
        return addRecordNative(name, data, offset, numBytes);
    }

    public byte[] getRecord(int recordId) throws RecordStoreNotOpenException, InvalidRecordIDException, RecordStoreException {
        if (nativePtr == 0) {
            throw new RecordStoreNotOpenException();
        }
        return getRecordNative(name, recordId);
    }

    public void deleteRecord(int recordId) throws RecordStoreNotOpenException, InvalidRecordIDException, RecordStoreException {
        if (nativePtr == 0) {
            throw new RecordStoreNotOpenException();
        }
        deleteRecordNative(name, recordId);
    }

    public int getNumRecords() throws RecordStoreNotOpenException {
        if (nativePtr == 0) {
            throw new RecordStoreNotOpenException();
        }
        return getNumRecordsNative(name);
    }

    public int getSize() throws RecordStoreNotOpenException {
        if (nativePtr == 0) {
            throw new RecordStoreNotOpenException();
        }
        return getSizeNative(name);
    }

    public int getSizeAvailable() throws RecordStoreNotOpenException {
        if (nativePtr == 0) {
            throw new RecordStoreNotOpenException();
        }
        return getSizeAvailableNative(name);
    }

    public void closeRecordStore() throws RecordStoreNotOpenException, RecordStoreException {
        if (nativePtr == 0) {
            throw new RecordStoreNotOpenException();
        }
        closeRecordStoreNative(name);
        nativePtr = 0;
    }

    public static void deleteRecordStore(String recordStoreName) throws RecordStoreException {
        deleteRecordStoreNative(recordStoreName);
    }

    public static String[] listRecordStores() {
        return listRecordStoresNative();
    }

    private static native int openRecordStoreNative(String name, boolean createIfNecessary);
    private native int addRecordNative(String name, byte[] data, int offset, int numBytes);
    private native byte[] getRecordNative(String name, int recordId);
    private native void deleteRecordNative(String name, int recordId);
    private native int getNumRecordsNative(String name);
    private native int getSizeNative(String name);
    private native int getSizeAvailableNative(String name);
    private native void closeRecordStoreNative(String name);
    private static native void deleteRecordStoreNative(String name);
    private static native String[] listRecordStoresNative();
}
