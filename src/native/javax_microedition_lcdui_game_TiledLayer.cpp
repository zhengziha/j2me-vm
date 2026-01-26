#include "javax_microedition_lcdui_game_TiledLayer.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "../core/Interpreter.hpp"
#include <iostream>

namespace j2me {
namespace natives {

void registerTiledLayerNatives(j2me::core::NativeRegistry& registry) {
    // TiledLayer.collidesWith(Sprite s, boolean pixelLevel)Z
    registry.registerNative("javax/microedition/lcdui/game/TiledLayer", "collidesWith", "(Ljavax/microedition/lcdui/game/Sprite;Z)Z", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue pixelLevelVal = frame->pop();
            j2me::core::JavaValue spriteVal = frame->pop();
            j2me::core::JavaValue thisVal = frame->pop();
            
            j2me::core::JavaObject* thisLayer = (j2me::core::JavaObject*)thisVal.val.ref;
            j2me::core::JavaObject* sprite = (j2me::core::JavaObject*)spriteVal.val.ref;
            
            if (!thisLayer || !sprite) {
                j2me::core::JavaValue result;
                result.type = j2me::core::JavaValue::INT;
                result.val.i = 0; // false
                frame->push(result);
                return;
            }
            
            // Get position and size from fields
            auto thisLayerClass = thisLayer->cls;
            auto spriteClass = sprite->cls;
            
            // Get x, y, width, height fields for TiledLayer
            auto thisXIt = thisLayerClass->fieldOffsets.find("x");
            auto thisYIt = thisLayerClass->fieldOffsets.find("y");
            auto thisWIt = thisLayerClass->fieldOffsets.find("width");
            auto thisHIt = thisLayerClass->fieldOffsets.find("height");
            
            // Get x, y, width, height fields for Sprite
            auto spriteXIt = spriteClass->fieldOffsets.find("x");
            auto spriteYIt = spriteClass->fieldOffsets.find("y");
            auto spriteWIt = spriteClass->fieldOffsets.find("width");
            auto spriteHIt = spriteClass->fieldOffsets.find("height");
            
            if (thisXIt == thisLayerClass->fieldOffsets.end() || thisYIt == thisLayerClass->fieldOffsets.end() ||
                thisWIt == thisLayerClass->fieldOffsets.end() || thisHIt == thisLayerClass->fieldOffsets.end() ||
                spriteXIt == spriteClass->fieldOffsets.end() || spriteYIt == spriteClass->fieldOffsets.end() ||
                spriteWIt == spriteClass->fieldOffsets.end() || spriteHIt == spriteClass->fieldOffsets.end()) {
                j2me::core::JavaValue result;
                result.type = j2me::core::JavaValue::INT;
                result.val.i = 0; // false
                frame->push(result);
                return;
            }
            
            int thisX = (int)thisLayer->fields[thisXIt->second];
            int thisY = (int)thisLayer->fields[thisYIt->second];
            int thisW = (int)thisLayer->fields[thisWIt->second];
            int thisH = (int)thisLayer->fields[thisHIt->second];
            
            int spriteX = (int)sprite->fields[spriteXIt->second];
            int spriteY = (int)sprite->fields[spriteYIt->second];
            int spriteW = (int)sprite->fields[spriteWIt->second];
            int spriteH = (int)sprite->fields[spriteHIt->second];
            
            // AABB collision detection
            bool collision = (thisX < spriteX + spriteW) && (thisX + thisW > spriteX) &&
                          (thisY < spriteY + spriteH) && (thisY + thisH > spriteY);
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::INT;
            result.val.i = collision ? 1 : 0;
            frame->push(result);
        });
}

} 
}
