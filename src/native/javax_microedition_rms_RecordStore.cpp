#include "javax_microedition_rms_RecordStore.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "../core/Interpreter.hpp"
#include "../core/Logger.hpp"
#include "../core/Diagnostics.hpp"
#include "java_lang_String.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <sys/stat.h>
#include <dirent.h>

namespace j2me {
namespace natives {

struct RecordStoreData {
    std::map<int, std::vector<uint8_t>> records;
    int nextRecordId;
    int size;
    int version;
    int64_t lastModifiedMs;
};

static std::map<std::string, RecordStoreData> g_recordStores;
static std::string g_rmsDir = "rms_data";

// Helper function to extract string from Java String object
std::string getStringFromJavaObject(j2me::core::JavaValue& value) {
    if (value.type == j2me::core::JavaValue::REFERENCE) {
        if (!value.strVal.empty()) {
            return value.strVal;
        } else if (value.val.ref != nullptr) {
            return j2me::natives::getJavaString(static_cast<j2me::core::JavaObject*>(value.val.ref));
        }
    }
    return "";
}

std::string getRecordStorePath(const std::string& name) {
    return g_rmsDir + "/" + name + ".dat";
}

void ensureRmsDir() {
    struct stat st;
    if (stat(g_rmsDir.c_str(), &st) != 0) {
        mkdir(g_rmsDir.c_str(), 0755);
    }
}

void saveRecordStore(const std::string& name) {
    std::string path = getRecordStorePath(name);
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR("[RMS] Failed to open file for writing: " + path);
        return;
    }

    auto& store = g_recordStores[name];
    file.write(reinterpret_cast<const char*>(&store.nextRecordId), sizeof(int));
    file.write(reinterpret_cast<const char*>(&store.size), sizeof(int));
    
    int numRecords = store.records.size();
    file.write(reinterpret_cast<const char*>(&numRecords), sizeof(int));
    
    for (const auto& entry : store.records) {
        int recordId = entry.first;
        int dataSize = entry.second.size();
        file.write(reinterpret_cast<const char*>(&recordId), sizeof(int));
        file.write(reinterpret_cast<const char*>(&dataSize), sizeof(int));
        file.write(reinterpret_cast<const char*>(entry.second.data()), dataSize);
    }
    
    file.close();
}

void loadRecordStore(const std::string& name) {
    std::string path = getRecordStorePath(name);
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return;
    }

    RecordStoreData store;
    file.read(reinterpret_cast<char*>(&store.nextRecordId), sizeof(int));
    file.read(reinterpret_cast<char*>(&store.size), sizeof(int));
    
    int numRecords;
    file.read(reinterpret_cast<char*>(&numRecords), sizeof(int));
    
    for (int i = 0; i < numRecords; i++) {
        int recordId, dataSize;
        file.read(reinterpret_cast<char*>(&recordId), sizeof(int));
        file.read(reinterpret_cast<char*>(&dataSize), sizeof(int));
        
        std::vector<uint8_t> data(dataSize);
        file.read(reinterpret_cast<char*>(data.data()), dataSize);
        store.records[recordId] = data;
    }
    
    g_recordStores[name] = store;
    file.close();
}

void registerRecordStoreNatives(j2me::core::NativeRegistry& registry) {
    ensureRmsDir();

    // Load existing record stores
    DIR* dir = opendir(g_rmsDir.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string filename = entry->d_name;
            if (filename.length() > 4 && filename.substr(filename.length() - 4) == ".dat") {
                std::string name = filename.substr(0, filename.length() - 4);
                loadRecordStore(name);
                LOG_INFO("[RMS] Loaded record store: " + name);
            }
        }
        closedir(dir);
    }

    // javax/microedition/rms/RecordStore.openRecordStoreNative(Ljava/lang/String;Z)I
    registry.registerNative("javax/microedition/rms/RecordStore", "openRecordStoreNative", "(Ljava/lang/String;Z)I",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue createVal = frame->pop();
            j2me::core::JavaValue nameVal = frame->pop();
            
            int result = 0;
            
            std::string name = getStringFromJavaObject(nameVal);

            if (!name.empty()) {
                bool createIfNecessary = (createVal.val.i != 0);
                
                LOG_DEBUG("[RMS] Opening record store: " + name + " (create: " + (createIfNecessary ? "true" : "false") + ")");
                
                if (g_recordStores.find(name) == g_recordStores.end()) {
                    RecordStoreData store;
                    store.nextRecordId = 1;
                    store.size = 0;
                    store.version = 0;
                    store.lastModifiedMs = 0;
                    g_recordStores[name] = store;
                    saveRecordStore(name);
                    LOG_DEBUG("[RMS] Created new record store: " + name + " (create flag: " + (createIfNecessary ? "true" : "false") + ")");
                }
                
                result = reinterpret_cast<intptr_t>(&g_recordStores[name]);
            }
            
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::INT;
            ret.val.i = result;
            frame->push(ret);
        }
    );

    // javax/microedition/rms/RecordStore.addRecordNative(Ljava/lang/String;[BII)I
    registry.registerNative("javax/microedition/rms/RecordStore", "addRecordNative", "(Ljava/lang/String;[BII)I",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue numBytesVal = frame->pop();
            j2me::core::JavaValue offsetVal = frame->pop();
            j2me::core::JavaValue dataVal = frame->pop();
            j2me::core::JavaValue nameVal = frame->pop();
            
            LOG_DEBUG("[RMS] addRecordNative called:");
            LOG_DEBUG("[RMS]   nameVal type=" + std::to_string(nameVal.type) + " ref=" + std::to_string(reinterpret_cast<uintptr_t>(nameVal.val.ref)) + " strVal=" + nameVal.strVal);
            LOG_DEBUG("[RMS]   dataVal type=" + std::to_string(dataVal.type) + " ref=" + std::to_string(reinterpret_cast<uintptr_t>(dataVal.val.ref)));
            LOG_DEBUG("[RMS]   offset=" + std::to_string(offsetVal.val.i) + " numBytes=" + std::to_string(numBytesVal.val.i));
            
            int recordId = 0;
            
            std::string name = getStringFromJavaObject(nameVal);
            
            if (!name.empty()) {
                LOG_DEBUG("[RMS]   Extracted name from String object: " + name);
            }
            
            if (!name.empty()) {
                if (dataVal.type == j2me::core::JavaValue::REFERENCE && dataVal.val.ref) {
                    auto dataObj = static_cast<j2me::core::JavaObject*>(dataVal.val.ref);
                    int offset = offsetVal.val.i;
                    int numBytes = numBytesVal.val.i;
                    
                    LOG_DEBUG("[RMS]   dataObj fields size=" + std::to_string(dataObj->fields.size()));
                    
                    std::vector<uint8_t> data;
                    for (int i = offset; i < offset + numBytes && i < (int)dataObj->fields.size(); i++) {
                        data.push_back(static_cast<uint8_t>(dataObj->fields[i]));
                    }
                    
                    auto it = g_recordStores.find(name);
                    if (it != g_recordStores.end()) {
                        recordId = it->second.nextRecordId++;
                        it->second.records[recordId] = data;
                        it->second.size += data.size();
                        it->second.version++;
                        it->second.lastModifiedMs = j2me::core::Diagnostics::getInstance().getNowMs();
                        saveRecordStore(name);
                        LOG_DEBUG("[RMS] Added record " + std::to_string(recordId) + " to " + name + " (size: " + std::to_string(data.size()) + ")");
                    }
                } else {
                    LOG_ERROR("[RMS] dataVal is not a valid reference");
                }
            } else {
                LOG_ERROR("[RMS] name is empty");
            }
            
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::INT;
            ret.val.i = recordId;
            frame->push(ret);
        }
    );

    // javax/microedition/rms/RecordStore.setRecordNative(Ljava/lang/String;I[BII)V
    registry.registerNative("javax/microedition/rms/RecordStore", "setRecordNative", "(Ljava/lang/String;I[BII)V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int numBytes = frame->pop().val.i;
            int offset = frame->pop().val.i;
            j2me::core::JavaValue dataVal = frame->pop();
            int recordId = frame->pop().val.i;
            j2me::core::JavaValue nameVal = frame->pop();

            std::string name = getStringFromJavaObject(nameVal);
            if (name.empty()) return;

            std::vector<uint8_t> data;
            if (dataVal.type == j2me::core::JavaValue::REFERENCE && dataVal.val.ref != nullptr) {
                auto dataObj = static_cast<j2me::core::JavaObject*>(dataVal.val.ref);
                if (dataObj) {
                    for (int i = offset; i < offset + numBytes && i < (int)dataObj->fields.size(); i++) {
                        data.push_back(static_cast<uint8_t>(dataObj->fields[i]));
                    }
                }
            }

            auto it = g_recordStores.find(name);
            if (it == g_recordStores.end()) {
                RecordStoreData store;
                store.nextRecordId = 1;
                store.size = 0;
                store.version = 0;
                store.lastModifiedMs = 0;
                it = g_recordStores.emplace(name, store).first;
            }

            auto& store = it->second;
            auto existing = store.records.find(recordId);
            if (existing != store.records.end()) {
                store.size -= (int)existing->second.size();
            }
            store.records[recordId] = data;
            store.size += (int)data.size();
            if (recordId >= store.nextRecordId) store.nextRecordId = recordId + 1;
            store.version++;
            store.lastModifiedMs = j2me::core::Diagnostics::getInstance().getNowMs();
            saveRecordStore(name);
        }
    );

    // javax/microedition/rms/RecordStore.getRecordSizeNative(Ljava/lang/String;I)I
    registry.registerNative("javax/microedition/rms/RecordStore", "getRecordSizeNative", "(Ljava/lang/String;I)I",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int recordId = frame->pop().val.i;
            j2me::core::JavaValue nameVal = frame->pop();

            int size = 0;
            std::string name = getStringFromJavaObject(nameVal);

            auto it = g_recordStores.find(name);
            if (it != g_recordStores.end()) {
                auto recIt = it->second.records.find(recordId);
                if (recIt != it->second.records.end()) size = (int)recIt->second.size();
            }

            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::INT;
            ret.val.i = size;
            frame->push(ret);
        }
    );

    // javax/microedition/rms/RecordStore.getRecordIdsNative(Ljava/lang/String;)[I
    registry.registerNative("javax/microedition/rms/RecordStore", "getRecordIdsNative", "(Ljava/lang/String;)[I",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue nameVal = frame->pop();

            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::REFERENCE;
            result.val.ref = nullptr;

            std::string name = getStringFromJavaObject(nameVal);

            auto it = g_recordStores.find(name);
            if (it != g_recordStores.end()) {
                auto interpreter = j2me::core::NativeRegistry::getInstance().getInterpreter();
                auto arrayCls = interpreter->resolveClass("[I");
                if (arrayCls) {
                    auto arrayObj = j2me::core::HeapManager::getInstance().allocate(arrayCls);
                    arrayObj->fields.resize(it->second.records.size());
                    size_t idx = 0;
                    for (const auto& rec : it->second.records) {
                        arrayObj->fields[idx++] = (int64_t)rec.first;
                    }
                    result.val.ref = arrayObj;
                }
            }
            frame->push(result);
        }
    );

    // javax/microedition/rms/RecordStore.getLastModifiedNative(Ljava/lang/String;)J
    registry.registerNative("javax/microedition/rms/RecordStore", "getLastModifiedNative", "(Ljava/lang/String;)J",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue nameVal = frame->pop();
            int64_t v = 0;
            std::string name = getStringFromJavaObject(nameVal);
            auto it = g_recordStores.find(name);
            if (it != g_recordStores.end()) v = it->second.lastModifiedMs;
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::LONG;
            ret.val.l = v;
            frame->push(ret);
        }
    );

    // javax/microedition/rms/RecordStore.getVersionNative(Ljava/lang/String;)I
    registry.registerNative("javax/microedition/rms/RecordStore", "getVersionNative", "(Ljava/lang/String;)I",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue nameVal = frame->pop();
            int v = 0;
            std::string name = getStringFromJavaObject(nameVal);
            auto it = g_recordStores.find(name);
            if (it != g_recordStores.end()) v = it->second.version;
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::INT;
            ret.val.i = v;
            frame->push(ret);
        }
    );

    // javax/microedition/rms/RecordStore.getRecordNative(Ljava/lang/String;I)[B
    registry.registerNative("javax/microedition/rms/RecordStore", "getRecordNative", "(Ljava/lang/String;I)[B",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue recordIdVal = frame->pop();
            j2me::core::JavaValue nameVal = frame->pop();
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::REFERENCE;
            result.val.ref = nullptr;
            
            std::string name = getStringFromJavaObject(nameVal);
            
            if (!name.empty()) {
                int recordId = recordIdVal.val.i;
                
                auto it = g_recordStores.find(name);
                if (it != g_recordStores.end()) {
                    auto recordIt = it->second.records.find(recordId);
                    if (recordIt != it->second.records.end()) {
                        auto& data = recordIt->second;
                        
                        auto interpreter = j2me::core::NativeRegistry::getInstance().getInterpreter();
                        auto arrayCls = interpreter->resolveClass("[B");
                        if (arrayCls) {
                            auto arrayObj = j2me::core::HeapManager::getInstance().allocate(arrayCls);
                            arrayObj->fields.resize(data.size());
                            for (size_t i = 0; i < data.size(); i++) {
                                arrayObj->fields[i] = data[i];
                            }
                            result.val.ref = arrayObj;
                            LOG_DEBUG("[RMS] Retrieved record " + std::to_string(recordId) + " from " + name + " (size: " + std::to_string(data.size()) + ")");
                        }
                    } else {
                        LOG_ERROR("[RMS] Record not found: " + std::to_string(recordId));
                    }
                }
            }
            
            frame->push(result);
        }
    );

    // javax/microedition/rms/RecordStore.deleteRecordNative(Ljava/lang/String;I)V
    registry.registerNative("javax/microedition/rms/RecordStore", "deleteRecordNative", "(Ljava/lang/String;I)V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue recordIdVal = frame->pop();
            j2me::core::JavaValue nameVal = frame->pop();
            
            std::string name = getStringFromJavaObject(nameVal);
            
            if (!name.empty()) {
                int recordId = recordIdVal.val.i;
                
                auto it = g_recordStores.find(name);
                if (it != g_recordStores.end()) {
                    auto recordIt = it->second.records.find(recordId);
                    if (recordIt != it->second.records.end()) {
                        it->second.size -= recordIt->second.size();
                        it->second.records.erase(recordIt);
                        it->second.version++;
                        it->second.lastModifiedMs = j2me::core::Diagnostics::getInstance().getNowMs();
                        saveRecordStore(name);
                        LOG_DEBUG("[RMS] Deleted record " + std::to_string(recordId) + " from " + name);
                    } else {
                        LOG_ERROR("[RMS] Record not found: " + std::to_string(recordId));
                    }
                }
            }
        }
    );

    // javax/microedition/rms/RecordStore.getNumRecordsNative(Ljava/lang/String;)I
    registry.registerNative("javax/microedition/rms/RecordStore", "getNumRecordsNative", "(Ljava/lang/String;)I",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue nameVal = frame->pop();
            
            int numRecords = 0;
            std::string name = getStringFromJavaObject(nameVal);
            
            if (!name.empty()) {
                auto it = g_recordStores.find(name);
                if (it != g_recordStores.end()) {
                    numRecords = it->second.records.size();
                }
            }
            
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::INT;
            ret.val.i = numRecords;
            frame->push(ret);
        }
    );

    // javax/microedition/rms/RecordStore.getSizeNative(Ljava/lang/String;)I
    registry.registerNative("javax/microedition/rms/RecordStore", "getSizeNative", "(Ljava/lang/String;)I",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue nameVal = frame->pop();
            
            int size = 0;
            std::string name = getStringFromJavaObject(nameVal);
            
            if (!name.empty()) {
                auto it = g_recordStores.find(name);
                if (it != g_recordStores.end()) {
                    size = it->second.size;
                }
            }
            
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::INT;
            ret.val.i = size;
            frame->push(ret);
        }
    );

    // javax/microedition/rms/RecordStore.getSizeAvailableNative(Ljava/lang/String;)I
    registry.registerNative("javax/microedition/rms/RecordStore", "getSizeAvailableNative", "(Ljava/lang/String;)I",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue nameVal = frame->pop();
            
            int available = 1024 * 1024;
            std::string name = getStringFromJavaObject(nameVal);
            
            if (!name.empty()) {
                auto it = g_recordStores.find(name);
                if (it != g_recordStores.end()) {
                    available = std::max(0, 1024 * 1024 - it->second.size);
                }
            }
            
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::INT;
            ret.val.i = available;
            frame->push(ret);
        }
    );

    // javax/microedition/rms/RecordStore.closeRecordStoreNative(Ljava/lang/String;)V
    registry.registerNative("javax/microedition/rms/RecordStore", "closeRecordStoreNative", "(Ljava/lang/String;)V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue nameVal = frame->pop();

            std::string name = getStringFromJavaObject(nameVal);
            if (!name.empty()) {
                saveRecordStore(name);
                LOG_DEBUG("[RMS] Closed record store: " + name);
            }
        }
    );

    // javax/microedition/rms/RecordStore.deleteRecordStoreNative(Ljava/lang/String;)V
    registry.registerNative("javax/microedition/rms/RecordStore", "deleteRecordStoreNative", "(Ljava/lang/String;)V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue nameVal = frame->pop();
            
            std::string name = getStringFromJavaObject(nameVal);
            if (!name.empty()) {
                auto it = g_recordStores.find(name);
                if (it != g_recordStores.end()) {
                    g_recordStores.erase(it);
                }
                
                std::string path = getRecordStorePath(name);
                std::remove(path.c_str());
                LOG_DEBUG("[RMS] Deleted record store: " + name);
            }
        }
    );

    // javax/microedition/rms/RecordStore.listRecordStoresNative()[Ljava/lang/String;
    registry.registerNative("javax/microedition/rms/RecordStore", "listRecordStoresNative", "()[Ljava/lang/String;",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::REFERENCE;
            result.val.ref = nullptr;
            
            if (!g_recordStores.empty()) {
                auto interpreter = j2me::core::NativeRegistry::getInstance().getInterpreter();
                auto arrayCls = interpreter->resolveClass("[Ljava/lang/String;");
                if (arrayCls) {
                    auto arrayObj = j2me::core::HeapManager::getInstance().allocate(arrayCls);
                    arrayObj->fields.resize(g_recordStores.size());
                    
                    int index = 0;
                    for (const auto& entry : g_recordStores) {
                        auto stringCls = interpreter->resolveClass("java/lang/String");
                        if (stringCls) {
                            auto stringObj = j2me::core::HeapManager::getInstance().allocate(stringCls);
                            
                            auto byteArrayCls = interpreter->resolveClass("[B");
                            if (byteArrayCls) {
                                auto byteArrayObj = j2me::core::HeapManager::getInstance().allocate(byteArrayCls);
                                byteArrayObj->fields.resize(entry.first.size());
                                for (size_t i = 0; i < entry.first.size(); i++) {
                                    byteArrayObj->fields[i] = entry.first[i];
                                }
                                
                                auto valueIt = stringObj->cls->fieldOffsets.find("value");
                                if (valueIt != stringObj->cls->fieldOffsets.end()) {
                                    stringObj->fields[valueIt->second] = reinterpret_cast<intptr_t>(byteArrayObj);
                                }
                                
                                auto offsetIt = stringObj->cls->fieldOffsets.find("offset");
                                if (offsetIt != stringObj->cls->fieldOffsets.end()) {
                                    stringObj->fields[offsetIt->second] = 0;
                                }
                                
                                auto countIt = stringObj->cls->fieldOffsets.find("count");
                                if (countIt != stringObj->cls->fieldOffsets.end()) {
                                    stringObj->fields[countIt->second] = entry.first.size();
                                }
                            }
                            
                            arrayObj->fields[index] = reinterpret_cast<intptr_t>(stringObj);
                            LOG_DEBUG("[RMS]   Record store: " + entry.first);
                            index++;
                        }
                    }
                    
                    result.val.ref = arrayObj;
                    LOG_DEBUG("[RMS] Listed " + std::to_string(g_recordStores.size()) + " record stores");
                }
            }
            
            frame->push(result);
        }
    );
}

}
}
