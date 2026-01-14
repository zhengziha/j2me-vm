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
        return addRecordNative(nativePtr, data, offset, numBytes);
    }

    public byte[] getRecord(int recordId) throws RecordStoreNotOpenException, InvalidRecordIDException, RecordStoreException {
        if (nativePtr == 0) {
            throw new RecordStoreNotOpenException();
        }
        return getRecordNative(nativePtr, recordId);
    }

    public void deleteRecord(int recordId) throws RecordStoreNotOpenException, InvalidRecordIDException, RecordStoreException {
        if (nativePtr == 0) {
            throw new RecordStoreNotOpenException();
        }
        deleteRecordNative(nativePtr, recordId);
    }

    public int getNumRecords() throws RecordStoreNotOpenException {
        if (nativePtr == 0) {
            throw new RecordStoreNotOpenException();
        }
        return getNumRecordsNative(nativePtr);
    }

    public int getSize() throws RecordStoreNotOpenException {
        if (nativePtr == 0) {
            throw new RecordStoreNotOpenException();
        }
        return getSizeNative(nativePtr);
    }

    public int getSizeAvailable() throws RecordStoreNotOpenException {
        if (nativePtr == 0) {
            throw new RecordStoreNotOpenException();
        }
        return getSizeAvailableNative(nativePtr);
    }

    public void closeRecordStore() throws RecordStoreNotOpenException, RecordStoreException {
        if (nativePtr == 0) {
            throw new RecordStoreNotOpenException();
        }
        closeRecordStoreNative(nativePtr);
        nativePtr = 0;
    }

    public static void deleteRecordStore(String recordStoreName) throws RecordStoreException {
        deleteRecordStoreNative(recordStoreName);
    }

    public static String[] listRecordStores() {
        return listRecordStoresNative();
    }

    private static native int openRecordStoreNative(String name, boolean createIfNecessary);
    private native int addRecordNative(int ptr, byte[] data, int offset, int numBytes);
    private native byte[] getRecordNative(int ptr, int recordId);
    private native void deleteRecordNative(int ptr, int recordId);
    private native int getNumRecordsNative(int ptr);
    private native int getSizeNative(int ptr);
    private native int getSizeAvailableNative(int ptr);
    private native void closeRecordStoreNative(int ptr);
    private static native void deleteRecordStoreNative(String name);
    private static native String[] listRecordStoresNative();
}
