package javax.microedition.rms;

public class RecordEnumerationImpl implements RecordEnumeration {
    private RecordStore recordStore;
    private RecordFilter filter;
    private RecordComparator comparator;
    private boolean keepUpdated;
    private int[] records;
    private int index;

    public RecordEnumerationImpl(RecordStore recordStore, RecordFilter filter, RecordComparator comparator, boolean keepUpdated) {
        this.recordStore = recordStore;
        this.filter = filter;
        this.comparator = comparator;
        this.keepUpdated = keepUpdated;
        this.records = null;
        this.index = 0;
        rebuild();
    }

    public void destroy() {
        this.recordStore = null;
        this.records = null;
    }

    public boolean hasNextElement() {
        if (records == null) return false;
        return index < records.length;
    }

    public boolean hasPreviousElement() {
        if (records == null) return false;
        return index > 0;
    }

    public boolean isKeptUpdated() {
        return keepUpdated;
    }

    public void keepUpdated(boolean keepUpdated) {
        this.keepUpdated = keepUpdated;
        if (keepUpdated) rebuild();
    }

    public byte[] nextRecord() throws RecordStoreNotOpenException, RecordStoreException, InvalidRecordIDException {
        if (!hasNextElement()) throw new InvalidRecordIDException();
        int id = records[index++];
        return recordStore.getRecord(id);
    }

    public int nextRecordId() throws InvalidRecordIDException {
        if (!hasNextElement()) throw new InvalidRecordIDException();
        return records[index++];
    }

    public int numRecords() {
        return records == null ? 0 : records.length;
    }

    public byte[] previousRecord() throws RecordStoreNotOpenException, RecordStoreException, InvalidRecordIDException {
        if (!hasPreviousElement()) throw new InvalidRecordIDException();
        int id = records[--index];
        return recordStore.getRecord(id);
    }

    public int previousRecordId() throws InvalidRecordIDException {
        if (!hasPreviousElement()) throw new InvalidRecordIDException();
        return records[--index];
    }

    public void rebuild() {
        if (recordStore == null) {
             this.records = new int[0];
             return;
        }
        
        int[] allIds = recordStore.getRecordIds();
        if (allIds == null) {
            this.records = new int[0];
            return;
        }

        // Filter and sort
        // For simplicity, we just copy all IDs for now if no filter/comparator
        // Implementing full filter/comparator logic in Java
        
        int count = 0;
        int[] temp = new int[allIds.length];
        
        for (int i = 0; i < allIds.length; i++) {
            int id = allIds[i];
            boolean match = true;
            if (filter != null) {
                try {
                    byte[] data = recordStore.getRecord(id);
                    match = filter.matches(data);
                } catch (Exception e) {
                    match = false;
                }
            }
            if (match) {
                temp[count++] = id;
            }
        }
        
        this.records = new int[count];
        System.arraycopy(temp, 0, this.records, 0, count);

        if (comparator != null) {
             // Simple bubble sort for now
             for (int i = 0; i < count - 1; i++) {
                 for (int j = 0; j < count - i - 1; j++) {
                     try {
                         byte[] rec1 = recordStore.getRecord(this.records[j]);
                         byte[] rec2 = recordStore.getRecord(this.records[j+1]);
                         if (comparator.compare(rec1, rec2) == RecordComparator.FOLLOWS) {
                             int swap = this.records[j];
                             this.records[j] = this.records[j+1];
                             this.records[j+1] = swap;
                         }
                     } catch (Exception e) {
                         // ignore
                     }
                 }
             }
        }
        
        this.index = 0;
    }

    public void reset() {
        this.index = 0;
    }
}
