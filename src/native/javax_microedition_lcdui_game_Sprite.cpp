#include "javax_microedition_lcdui_game_Sprite.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/StackFrame.hpp"
#include "../core/HeapManager.hpp"
#include "../core/Interpreter.hpp"
#include <iostream>

namespace j2me {
namespace natives {

void registerSpriteNatives(j2me::core::NativeRegistry& registry) {
    // Sprite.collidesWithNative(Sprite s)Z
    registry.registerNative("javax/microedition/lcdui/game/Sprite", "collidesWithNative", "(Ljavax/microedition/lcdui/game/Sprite;)Z", 
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue spriteVal = frame->pop();
            j2me::core::JavaValue thisVal = frame->pop();
            
            j2me::core::JavaObject* thisSprite = (j2me::core::JavaObject*)thisVal.val.ref;
            j2me::core::JavaObject* otherSprite = (j2me::core::JavaObject*)spriteVal.val.ref;
            
            if (!thisSprite || !otherSprite) {
                j2me::core::JavaValue result;
                result.type = j2me::core::JavaValue::INT;
                result.val.i = 0; // false
                frame->push(result);
                return;
            }
            
            // Get position and size from fields
            auto thisSpriteClass = thisSprite->cls;
            auto otherSpriteClass = otherSprite->cls;
            
            // Get x, y, width, height fields
            auto thisXIt = thisSpriteClass->fieldOffsets.find("x");
            auto thisYIt = thisSpriteClass->fieldOffsets.find("y");
            auto thisWIt = thisSpriteClass->fieldOffsets.find("width");
            auto thisHIt = thisSpriteClass->fieldOffsets.find("height");
            
            auto otherXIt = otherSpriteClass->fieldOffsets.find("x");
            auto otherYIt = otherSpriteClass->fieldOffsets.find("y");
            auto otherWIt = otherSpriteClass->fieldOffsets.find("width");
            auto otherHIt = otherSpriteClass->fieldOffsets.find("height");
            
            if (thisXIt == thisSpriteClass->fieldOffsets.end() || thisYIt == thisSpriteClass->fieldOffsets.end() ||
                thisWIt == thisSpriteClass->fieldOffsets.end() || thisHIt == thisSpriteClass->fieldOffsets.end() ||
                otherXIt == otherSpriteClass->fieldOffsets.end() || otherYIt == otherSpriteClass->fieldOffsets.end() ||
                otherWIt == otherSpriteClass->fieldOffsets.end() || otherHIt == otherSpriteClass->fieldOffsets.end()) {
                j2me::core::JavaValue result;
                result.type = j2me::core::JavaValue::INT;
                result.val.i = 0; // false
                frame->push(result);
                return;
            }
            
            int thisX = (int)thisSprite->fields[thisXIt->second];
            int thisY = (int)thisSprite->fields[thisYIt->second];
            int thisW = (int)thisSprite->fields[thisWIt->second];
            int thisH = (int)thisSprite->fields[thisHIt->second];
            
            int otherX = (int)otherSprite->fields[otherXIt->second];
            int otherY = (int)otherSprite->fields[otherYIt->second];
            int otherW = (int)otherSprite->fields[otherWIt->second];
            int otherH = (int)otherSprite->fields[otherHIt->second];
            
            // AABB collision detection
            bool collision = (thisX < otherX + otherW) && (thisX + thisW > otherX) &&
                          (thisY < otherY + otherH) && (thisY + thisH > otherY);
            
            j2me::core::JavaValue result;
            result.type = j2me::core::JavaValue::INT;
            result.val.i = collision ? 1 : 0;
            frame->push(result);
        });
}

} 
}
