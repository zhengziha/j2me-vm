#include "javax_microedition_io_Connector.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "java_lang_String.hpp"

namespace j2me {
namespace natives {

void registerConnectorNatives(j2me::core::NativeRegistry& registry) {
    registry.registerNative("javax/microedition/io/Connector", "openNative", "(Ljava/lang/String;IZ)Ljavax/microedition/io/Connection;",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int timeouts = frame->pop().val.i;
            int mode = frame->pop().val.i;
            j2me::core::JavaValue nameVal = frame->pop();

            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::REFERENCE;
            result.val.ref = nullptr;

            auto interpreter = j2me::core::NativeRegistry::getInstance().getInterpreter();
            if (!interpreter) {
                frame->push(result);
                return;
            }

            std::string name;
            if (nameVal.type == j2me::core::JavaValue::REFERENCE) {
                if (!nameVal.strVal.empty()) {
                    name = nameVal.strVal;
                } else if (nameVal.val.ref != nullptr) {
                    name = j2me::natives::getJavaString(static_cast<j2me::core::JavaObject*>(nameVal.val.ref));
                }
            }

            if (name.rfind("sms:", 0) == 0 || name.rfind("sms://", 0) == 0) {
                auto connCls = interpreter->resolveClass("javax/wireless/messaging/MockMessageConnection");
                if (!connCls) {
                    frame->push(result);
                    return;
                }

                auto connObj = j2me::core::HeapManager::getInstance().allocate(connCls);
                if (!connObj) {
                    frame->push(result);
                    return;
                }

                auto addrStrObj = j2me::natives::createJavaString(interpreter, name);
                auto addrField = connCls->fieldOffsets.find("address|Ljava/lang/String;");
                if (addrField == connCls->fieldOffsets.end()) addrField = connCls->fieldOffsets.find("address");
                if (addrField != connCls->fieldOffsets.end()) {
                    connObj->fields[addrField->second] = (int64_t)addrStrObj;
                }

                result.val.ref = connObj;
                frame->push(result);
                return;
            }

            frame->push(result);
        }
    );
}

} // namespace natives
} // namespace j2me
