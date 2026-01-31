#include "javax_microedition_media_Manager.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "../core/Interpreter.hpp"
#include "../core/Logger.hpp"

namespace j2me {
namespace natives {

void registerMediaNatives(j2me::core::NativeRegistry& registry) {
    // javax/microedition/media/Manager.createPlayer(Ljava/io/InputStream;Ljava/lang/String;)Ljavax/microedition/media/Player;
    registry.registerNative("javax/microedition/media/Manager", "createPlayer", "(Ljava/io/InputStream;Ljava/lang/String;)Ljavax/microedition/media/Player;", 
        [&registry](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // type
            frame->pop(); // stream
            
            // Create AudioPlayer
            auto interpreter = registry.getInterpreter();
            if (interpreter) {
                auto playerCls = interpreter->resolveClass("j2me/media/AudioPlayer");
                if (playerCls) {
                    if (interpreter->initializeClass(thread, playerCls)) {
                        // Initialization pushed, we need to retry this native instruction later... 
                        // But native calls are immediate. 
                        // Ideally we should have initialized AudioPlayer at startup or lazily properly.
                        // For now, assume it's simple enough not to need complex <clinit>.
                        // Or we force it.
                    }
                    
                    auto playerObj = j2me::core::HeapManager::getInstance().allocate(playerCls);
                    j2me::core::JavaValue result;
                    result.type = j2me::core::JavaValue::REFERENCE;
                    result.val.ref = playerObj;
                    frame->push(result);
                    return;
                }
            }
            
            LOG_ERROR("[Media] Failed to create AudioPlayer (Stream)");
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::REFERENCE;
            result.val.ref = nullptr;
            frame->push(result);
        }
    );

    // javax/microedition/media/Manager.createPlayer(Ljava/lang/String;)Ljavax/microedition/media/Player;
    registry.registerNative("javax/microedition/media/Manager", "createPlayer", "(Ljava/lang/String;)Ljavax/microedition/media/Player;", 
        [&registry](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // locator
            
            auto interpreter = registry.getInterpreter();
            if (interpreter) {
                auto playerCls = interpreter->resolveClass("j2me/media/AudioPlayer");
                if (playerCls) {
                    if (interpreter->initializeClass(thread, playerCls)) {
                         // See above note
                    }

                    auto playerObj = j2me::core::HeapManager::getInstance().allocate(playerCls);
                    LOG_DEBUG("[Media] Created AudioPlayer instance: " + std::to_string((intptr_t)playerObj));
                    j2me::core::JavaValue result;
                    result.type = j2me::core::JavaValue::REFERENCE;
                    result.val.ref = playerObj;
                    frame->push(result);
                    return;
                }
            }
            
            LOG_ERROR("[Media] Failed to create AudioPlayer (Locator)");
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::REFERENCE;
            result.val.ref = nullptr;
            frame->push(result);
        }
    );
}

} // namespace natives
} // namespace j2me
