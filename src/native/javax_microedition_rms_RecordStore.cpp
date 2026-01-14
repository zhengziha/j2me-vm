#include "javax_microedition_rms_RecordStore.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "../core/Interpreter.hpp"
#include "../core/Logger.hpp"
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
};

static std::map<std::string, RecordStoreData> g_recordStores;
static std::string g_rmsDir = "rms_data";

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

void registerRecordStoreNatives() {
    auto& registry = j2me::core::NativeRegistry::getInstance();
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
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue createVal = frame->pop();
            j2me::core::JavaValue nameVal = frame->pop();
            
            int result = 0;
            
            if (nameVal.type == j2me::core::JavaValue::REFERENCE && !nameVal.strVal.empty()) {
                std::string name = nameVal.strVal;
                bool createIfNecessary = (createVal.val.i != 0);
                
                LOG_DEBUG("[RMS] Opening record store: " + name + " (create: " + (createIfNecessary ? "true" : "false") + ")");
                
                if (g_recordStores.find(name) == g_recordStores.end()) {
                    if (createIfNecessary) {
                        RecordStoreData store;
                        store.nextRecordId = 1;
                        store.size = 0;
                        g_recordStores[name] = store;
                        saveRecordStore(name);
                        LOG_DEBUG("[RMS] Created new record store: " + name);
                    } else {
                        LOG_ERROR("[RMS] Record store not found: " + name);
                        j2me::core::JavaValue ret;
                        ret.type = j2me::core::JavaValue::INT;
                        ret.val.i = 0;
                        frame->push(ret);
                        return;
                    }
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
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue numBytesVal = frame->pop();
            j2me::core::JavaValue offsetVal = frame->pop();
            j2me::core::JavaValue dataVal = frame->pop();
            j2me::core::JavaValue nameVal = frame->pop();
            
            LOG_DEBUG("[RMS] addRecordNative called:");
            LOG_DEBUG("[RMS]   nameVal type=" + std::to_string(nameVal.type) + " ref=" + std::to_string(reinterpret_cast<uintptr_t>(nameVal.val.ref)) + " strVal=" + nameVal.strVal);
            LOG_DEBUG("[RMS]   dataVal type=" + std::to_string(dataVal.type) + " ref=" + std::to_string(reinterpret_cast<uintptr_t>(dataVal.val.ref)));
            LOG_DEBUG("[RMS]   offset=" + std::to_string(offsetVal.val.i) + " numBytes=" + std::to_string(numBytesVal.val.i));
            
            int recordId = 0;
            
            std::string name;
            
            if (nameVal.type == j2me::core::JavaValue::REFERENCE && nameVal.val.ref != nullptr) {
                if (!nameVal.strVal.empty()) {
                    name = nameVal.strVal;
                    LOG_DEBUG("[RMS]   Using strVal directly: " + name);
                } else {
                    auto nameObj = static_cast<j2me::core::JavaObject*>(nameVal.val.ref);
                    LOG_DEBUG("[RMS]   nameObj=" + std::to_string(reinterpret_cast<uintptr_t>(nameObj)) + " cls=" + std::to_string(reinterpret_cast<uintptr_t>(nameObj ? nameObj->cls.get() : nullptr)));
                    if (nameObj && nameObj->cls) {
                        LOG_DEBUG("[RMS]   cls name=" + nameObj->cls->name + " fieldOffsets size=" + std::to_string(nameObj->cls->fieldOffsets.size()));
                        auto valueIt = nameObj->cls->fieldOffsets.find("value");
                        if (valueIt != nameObj->cls->fieldOffsets.end()) {
                            LOG_DEBUG("[RMS]   Found 'value' field at offset " + std::to_string(valueIt->second));
                            auto valueArray = static_cast<j2me::core::JavaObject*>((void*)nameObj->fields[valueIt->second]);
                            LOG_DEBUG("[RMS]   valueArray=" + std::to_string(reinterpret_cast<uintptr_t>(valueArray)));
                            if (valueArray) {
                                auto offsetIt = nameObj->cls->fieldOffsets.find("offset");
                                auto countIt = nameObj->cls->fieldOffsets.find("count");
                                int offset = (offsetIt != nameObj->cls->fieldOffsets.end()) ? (int)nameObj->fields[offsetIt->second] : 0;
                                int count = (countIt != nameObj->cls->fieldOffsets.end()) ? (int)nameObj->fields[countIt->second] : valueArray->fields.size();
                                
                                LOG_DEBUG("[RMS]   offset=" + std::to_string(offset) + " count=" + std::to_string(count) + " fields size=" + std::to_string(valueArray->fields.size()));
                                
                                for (int i = offset; i < offset + count && i < (int)valueArray->fields.size(); i++) {
                                    name += (char)valueArray->fields[i];
                                }
                                LOG_DEBUG("[RMS]   Extracted name from String object: " + name);
                            }
                        } else {
                            LOG_DEBUG("[RMS]   'value' field not found in String class");
                        }
                    }
                }
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

    // javax/microedition/rms/RecordStore.getRecordNative(Ljava/lang/String;I)[B
    registry.registerNative("javax/microedition/rms/RecordStore", "getRecordNative", "(Ljava/lang/String;I)[B",
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue recordIdVal = frame->pop();
            j2me::core::JavaValue nameVal = frame->pop();
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::REFERENCE;
            result.val.ref = nullptr;
            
            std::string name;
            
            if (nameVal.type == j2me::core::JavaValue::REFERENCE && nameVal.val.ref != nullptr) {
                if (!nameVal.strVal.empty()) {
                    name = nameVal.strVal;
                } else {
                    auto nameObj = static_cast<j2me::core::JavaObject*>(nameVal.val.ref);
                    if (nameObj && nameObj->cls) {
                        auto valueIt = nameObj->cls->fieldOffsets.find("value");
                        if (valueIt != nameObj->cls->fieldOffsets.end()) {
                            auto valueArray = static_cast<j2me::core::JavaObject*>((void*)nameObj->fields[valueIt->second]);
                            if (valueArray) {
                                auto offsetIt = nameObj->cls->fieldOffsets.find("offset");
                                auto countIt = nameObj->cls->fieldOffsets.find("count");
                                int offset = (offsetIt != nameObj->cls->fieldOffsets.end()) ? (int)nameObj->fields[offsetIt->second] : 0;
                                int count = (countIt != nameObj->cls->fieldOffsets.end()) ? (int)nameObj->fields[countIt->second] : valueArray->fields.size();
                                
                                for (int i = offset; i < offset + count && i < (int)valueArray->fields.size(); i++) {
                                    name += (char)valueArray->fields[i];
                                }
                            }
                        }
                    }
                }
            }
            
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
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue recordIdVal = frame->pop();
            j2me::core::JavaValue nameVal = frame->pop();
            
            std::string name;
            
            if (nameVal.type == j2me::core::JavaValue::REFERENCE && nameVal.val.ref != nullptr) {
                if (!nameVal.strVal.empty()) {
                    name = nameVal.strVal;
                } else {
                    auto nameObj = static_cast<j2me::core::JavaObject*>(nameVal.val.ref);
                    if (nameObj && nameObj->cls) {
                        auto valueIt = nameObj->cls->fieldOffsets.find("value");
                        if (valueIt != nameObj->cls->fieldOffsets.end()) {
                            auto valueArray = static_cast<j2me::core::JavaObject*>((void*)nameObj->fields[valueIt->second]);
                            if (valueArray) {
                                auto offsetIt = nameObj->cls->fieldOffsets.find("offset");
                                auto countIt = nameObj->cls->fieldOffsets.find("count");
                                int offset = (offsetIt != nameObj->cls->fieldOffsets.end()) ? (int)nameObj->fields[offsetIt->second] : 0;
                                int count = (countIt != nameObj->cls->fieldOffsets.end()) ? (int)nameObj->fields[countIt->second] : valueArray->fields.size();
                                
                                for (int i = offset; i < offset + count && i < (int)valueArray->fields.size(); i++) {
                                    name += (char)valueArray->fields[i];
                                }
                            }
                        }
                    }
                }
            }
            
            if (!name.empty()) {
                int recordId = recordIdVal.val.i;
                
                auto it = g_recordStores.find(name);
                if (it != g_recordStores.end()) {
                    auto recordIt = it->second.records.find(recordId);
                    if (recordIt != it->second.records.end()) {
                        it->second.size -= recordIt->second.size();
                        it->second.records.erase(recordIt);
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
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue nameVal = frame->pop();
            
            int numRecords = 0;
            std::string name;
            
            if (nameVal.type == j2me::core::JavaValue::REFERENCE && nameVal.val.ref != nullptr) {
                if (!nameVal.strVal.empty()) {
                    name = nameVal.strVal;
                } else {
                    auto nameObj = static_cast<j2me::core::JavaObject*>(nameVal.val.ref);
                    if (nameObj && nameObj->cls) {
                        auto valueIt = nameObj->cls->fieldOffsets.find("value");
                        if (valueIt != nameObj->cls->fieldOffsets.end()) {
                            auto valueArray = static_cast<j2me::core::JavaObject*>((void*)nameObj->fields[valueIt->second]);
                            if (valueArray) {
                                auto offsetIt = nameObj->cls->fieldOffsets.find("offset");
                                auto countIt = nameObj->cls->fieldOffsets.find("count");
                                int offset = (offsetIt != nameObj->cls->fieldOffsets.end()) ? (int)nameObj->fields[offsetIt->second] : 0;
                                int count = (countIt != nameObj->cls->fieldOffsets.end()) ? (int)nameObj->fields[countIt->second] : valueArray->fields.size();
                                
                                for (int i = offset; i < offset + count && i < (int)valueArray->fields.size(); i++) {
                                    name += (char)valueArray->fields[i];
                                }
                            }
                        }
                    }
                }
            }
            
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
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue nameVal = frame->pop();
            
            int size = 0;
            std::string name;
            
            if (nameVal.type == j2me::core::JavaValue::REFERENCE && nameVal.val.ref != nullptr) {
                if (!nameVal.strVal.empty()) {
                    name = nameVal.strVal;
                } else {
                    auto nameObj = static_cast<j2me::core::JavaObject*>(nameVal.val.ref);
                    if (nameObj && nameObj->cls) {
                        auto valueIt = nameObj->cls->fieldOffsets.find("value");
                        if (valueIt != nameObj->cls->fieldOffsets.end()) {
                            auto valueArray = static_cast<j2me::core::JavaObject*>((void*)nameObj->fields[valueIt->second]);
                            if (valueArray) {
                                auto offsetIt = nameObj->cls->fieldOffsets.find("offset");
                                auto countIt = nameObj->cls->fieldOffsets.find("count");
                                int offset = (offsetIt != nameObj->cls->fieldOffsets.end()) ? (int)nameObj->fields[offsetIt->second] : 0;
                                int count = (countIt != nameObj->cls->fieldOffsets.end()) ? (int)nameObj->fields[countIt->second] : valueArray->fields.size();
                                
                                for (int i = offset; i < offset + count && i < (int)valueArray->fields.size(); i++) {
                                    name += (char)valueArray->fields[i];
                                }
                            }
                        }
                    }
                }
            }
            
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
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue nameVal = frame->pop();
            
            int available = 1024 * 1024;
            std::string name;
            
            if (nameVal.type == j2me::core::JavaValue::REFERENCE && nameVal.val.ref != nullptr) {
                if (!nameVal.strVal.empty()) {
                    name = nameVal.strVal;
                } else {
                    auto nameObj = static_cast<j2me::core::JavaObject*>(nameVal.val.ref);
                    if (nameObj && nameObj->cls) {
                        auto valueIt = nameObj->cls->fieldOffsets.find("value");
                        if (valueIt != nameObj->cls->fieldOffsets.end()) {
                            auto valueArray = static_cast<j2me::core::JavaObject*>((void*)nameObj->fields[valueIt->second]);
                            if (valueArray) {
                                auto offsetIt = nameObj->cls->fieldOffsets.find("offset");
                                auto countIt = nameObj->cls->fieldOffsets.find("count");
                                int offset = (offsetIt != nameObj->cls->fieldOffsets.end()) ? (int)nameObj->fields[offsetIt->second] : 0;
                                int count = (countIt != nameObj->cls->fieldOffsets.end()) ? (int)nameObj->fields[countIt->second] : valueArray->fields.size();
                                
                                for (int i = offset; i < offset + count && i < (int)valueArray->fields.size(); i++) {
                                    name += (char)valueArray->fields[i];
                                }
                            }
                        }
                    }
                }
            }
            
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
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue nameVal = frame->pop();
            
            if (nameVal.type == j2me::core::JavaValue::REFERENCE && !nameVal.strVal.empty()) {
                std::string name = nameVal.strVal;
                saveRecordStore(name);
                LOG_DEBUG("[RMS] Closed record store: " + name);
            }
        }
    );

    // javax/microedition/rms/RecordStore.deleteRecordStoreNative(Ljava/lang/String;)V
    registry.registerNative("javax/microedition/rms/RecordStore", "deleteRecordStoreNative", "(Ljava/lang/String;)V",
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue nameVal = frame->pop();
            
            if (nameVal.type == j2me::core::JavaValue::REFERENCE && !nameVal.strVal.empty()) {
                std::string name = nameVal.strVal;
                
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
        [](std::shared_ptr<j2me::core::StackFrame> frame) {
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
