#include "javax_microedition_lcdui_Display.hpp"
#include "../core/NativeRegistry.hpp"
#include "../core/Logger.hpp"
#include "../platform/GraphicsContext.hpp"
#include <iostream>

namespace j2me {
namespace natives {

static j2me::core::JavaObject* g_currentDisplayable = nullptr;
static RepaintRequest g_repaintRequest;
static j2me::core::JavaObject* g_lastShownDisplayable = nullptr; // 记录上次显示的 Displayable

j2me::core::JavaObject* getCurrentDisplayable() {
    return g_currentDisplayable;
}

RepaintRequest& getRepaintRequest() {
    return g_repaintRequest;
}

// serviceRepaints - 立即执行挂起的重绘
void serviceRepaints(j2me::core::Interpreter* interpreter) {
    if (!g_repaintRequest.pending) {
        LOG_INFO("[serviceRepaints] No pending repaint request");
        return;
    }
    if (!g_currentDisplayable) {
        LOG_INFO("[serviceRepaints] No current displayable");
        return;
    }

    LOG_INFO("[serviceRepaints] Executing paint for displayable: " + g_currentDisplayable->cls->name);

    // 执行 paint 调用
    static std::shared_ptr<j2me::core::JavaClass> graphicsCls;
    if (!graphicsCls) {
        graphicsCls = interpreter->resolveClass("javax/microedition/lcdui/Graphics");
    }

    if (graphicsCls) {
        auto& gc = j2me::platform::GraphicsContext::getInstance();
        gc.beginFrame(); // 清空后缓冲区

        // 分配 Graphics 对象
        j2me::core::JavaObject* g = j2me::core::HeapManager::getInstance().allocate(graphicsCls);

        // 初始化 Graphics 对象字段
        gc.resetClip();
        int w = gc.getWidth();
        int h = gc.getHeight();

        auto setIntField = [&](const std::string& name, int val) {
            auto it = graphicsCls->fieldOffsets.find(name + "|I");
            if (it == graphicsCls->fieldOffsets.end()) it = graphicsCls->fieldOffsets.find(name);
            if (it != graphicsCls->fieldOffsets.end()) {
                g->fields[it->second] = val;
            }
        };

        setIntField("clipX", g_repaintRequest.fullScreen ? 0 : g_repaintRequest.x);
        setIntField("clipY", g_repaintRequest.fullScreen ? 0 : g_repaintRequest.y);
        setIntField("clipWidth", g_repaintRequest.fullScreen ? w : g_repaintRequest.width);
        setIntField("clipHeight", g_repaintRequest.fullScreen ? h : g_repaintRequest.height);
        setIntField("color", 0); // Black

        // 查找并调用 paint 方法
        auto currentCls = g_currentDisplayable->cls;
        bool paintFound = false;
        while (currentCls) {
            for (const auto& method : currentCls->rawFile->methods) {
                auto name = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(currentCls->rawFile->constant_pool[method.name_index]);
                if (name->bytes == "paint") {
                    LOG_INFO("[serviceRepaints] Found paint method in class: " + currentCls->name);
                    auto frame = std::make_shared<j2me::core::StackFrame>(method, currentCls->rawFile);

                    j2me::core::JavaValue vThis;
                    vThis.type = j2me::core::JavaValue::REFERENCE;
                    vThis.val.ref = g_currentDisplayable;
                    frame->setLocal(0, vThis);

                    j2me::core::JavaValue vG;
                    vG.type = j2me::core::JavaValue::REFERENCE;
                    vG.val.ref = g;
                    frame->setLocal(1, vG);

                    // 直接在同一线程执行 paint (J2ME 规范)
                    // 创建临时线程来执行 paint 方法
                    auto paintThread = std::make_shared<core::JavaThread>(frame);
                    interpreter->execute(paintThread, 100000);
                    paintFound = true;
                    break;
                }
            }
            if (paintFound) break;
            currentCls = currentCls->superClass;
        }

        if (!paintFound) {
            LOG_INFO("[serviceRepaints] No paint method found!");
        }

        // paint 完成后提交并显示
        gc.commit();
        gc.update();

        g_repaintRequest.clear();
        LOG_INFO("[serviceRepaints] Paint completed, committed and updated");
    } else {
        LOG_INFO("[serviceRepaints] Graphics class not found!");
    }
}

void registerDisplayNatives(j2me::core::NativeRegistry& registry) {
    // javax/microedition/lcdui/Canvas.repaintNative(IIII)V
    registry.registerNative("javax/microedition/lcdui/Canvas", "repaintNative", "(IIII)V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int h = frame->pop().val.i;
            int w = frame->pop().val.i;
            int y = frame->pop().val.i;
            int x = frame->pop().val.i;
            frame->pop(); // this
            g_repaintRequest.request(x, y, w, h);
        }
    );

    // javax/microedition/lcdui/Canvas.repaint()V
    registry.registerNative("javax/microedition/lcdui/Canvas", "repaint", "()V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // this
            g_repaintRequest.requestFull();
        }
    );

    // javax/microedition/lcdui/Canvas.repaint(IIII)V
    registry.registerNative("javax/microedition/lcdui/Canvas", "repaint", "(IIII)V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            int h = frame->pop().val.i;
            int w = frame->pop().val.i;
            int y = frame->pop().val.i;
            int x = frame->pop().val.i;
            frame->pop(); // this
            g_repaintRequest.request(x, y, w, h);
        }
    );

    // javax/microedition/lcdui/Canvas.serviceRepaintsNative()V
    registry.registerNative("javax/microedition/lcdui/Canvas", "serviceRepaintsNative", "()V",
        [&registry](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // this
            serviceRepaints(registry.getInterpreter());
        }
    );

    // javax/microedition/lcdui/Canvas.serviceRepaints()V
    registry.registerNative("javax/microedition/lcdui/Canvas", "serviceRepaints", "()V",
        [&registry](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // this
            serviceRepaints(registry.getInterpreter());
        }
    );

    // javax/microedition/lcdui/Display.serviceRepaints()V
    registry.registerNative("javax/microedition/lcdui/Display", "serviceRepaints", "()V",
        [&registry](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // this
            serviceRepaints(registry.getInterpreter());
        }
    );

    // javax/microedition/lcdui/Canvas.setFullScreenMode(Z)V
    registry.registerNative("javax/microedition/lcdui/Canvas", "setFullScreenMode", "(Z)V",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            bool mode = frame->pop().val.i != 0;
            frame->pop(); // this
            LOG_INFO("[Canvas] setFullScreenMode: " + std::string(mode ? "true" : "false"));
        }
    );

    // javax/microedition/lcdui/Displayable.getWidth()I
    registry.registerNative("javax/microedition/lcdui/Displayable", "getWidth", "()I",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // this
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::INT;
            ret.val.i = 240;
            frame->push(ret);
        }
    );

    // javax/microedition/lcdui/Displayable.getHeight()I
    registry.registerNative("javax/microedition/lcdui/Displayable", "getHeight", "()I",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // this
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::INT;
            ret.val.i = 320;
            frame->push(ret);
        }
    );

    // Also register on Canvas just in case
    registry.registerNative("javax/microedition/lcdui/Canvas", "getWidth", "()I",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // this
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::INT;
            ret.val.i = 240;
            frame->push(ret);
        }
    );
    registry.registerNative("javax/microedition/lcdui/Canvas", "getHeight", "()I",
        [](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            frame->pop(); // this
            j2me::core::JavaValue ret;
            ret.type = j2me::core::JavaValue::INT;
            ret.val.i = 320;
            frame->push(ret);
        }
    );

    // javax/microedition/lcdui/Display.setCurrentNative(Ljavax/microedition/lcdui/Displayable;)V
    registry.registerNative("javax/microedition/lcdui/Display", "setCurrentNative", "(Ljavax/microedition/lcdui/Displayable;)V",
        [&registry](std::shared_ptr<j2me::core::JavaThread> thread, std::shared_ptr<j2me::core::StackFrame> frame) {
            j2me::core::JavaValue displayableVal = frame->pop();
            frame->pop(); // this (Display instance)

            j2me::core::JavaObject* oldDisplayable = g_currentDisplayable;
            g_currentDisplayable = (j2me::core::JavaObject*)displayableVal.val.ref;

            if (g_currentDisplayable && g_currentDisplayable->cls) {
                LOG_INFO("[Display] setCurrent: " + g_currentDisplayable->cls->name);
            } else {
                LOG_INFO("[Display] setCurrent: <null>");
            }

            // J2ME 规范：先调用旧 Displayable 的 hideNotify()
            if (oldDisplayable && oldDisplayable != g_currentDisplayable) {
                auto currentCls = oldDisplayable->cls;
                while (currentCls) {
                    for (const auto& method : currentCls->rawFile->methods) {
                        auto name = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(currentCls->rawFile->constant_pool[method.name_index]);
                        if (name->bytes == "hideNotify") {
                            LOG_INFO("[Display] Calling hideNotify on: " + oldDisplayable->cls->name);
                            auto stackFrame = std::make_shared<j2me::core::StackFrame>(method, currentCls->rawFile);

                            j2me::core::JavaValue vThis;
                            vThis.type = j2me::core::JavaValue::REFERENCE;
                            vThis.val.ref = oldDisplayable;
                            stackFrame->setLocal(0, vThis);

                            auto hideNotifyThread = std::make_shared<core::JavaThread>(stackFrame);
                            registry.getInterpreter()->execute(hideNotifyThread, 10000);
                            goto hideNotifyDone;
                        }
                    }
                    currentCls = currentCls->superClass;
                }
                hideNotifyDone:;
            }

            // J2ME 规范：setCurrent 时调用新 Displayable 的 showNotify()
            // J2ME 规范：setCurrent 时触发重绘
            // 检查是否是新的 Displayable，或者是首次显示
            if (g_currentDisplayable && g_currentDisplayable != g_lastShownDisplayable) {
                // 调用 showNotify()
                auto currentCls = g_currentDisplayable->cls;
                while (currentCls) {
                    for (const auto& method : currentCls->rawFile->methods) {
                        auto name = std::dynamic_pointer_cast<j2me::core::ConstantUtf8>(currentCls->rawFile->constant_pool[method.name_index]);
                        if (name->bytes == "showNotify") {
                            LOG_INFO("[Display] Calling showNotify on: " + g_currentDisplayable->cls->name);
                            auto stackFrame = std::make_shared<j2me::core::StackFrame>(method, currentCls->rawFile);

                            j2me::core::JavaValue vThis;
                            vThis.type = j2me::core::JavaValue::REFERENCE;
                            vThis.val.ref = g_currentDisplayable;
                            stackFrame->setLocal(0, vThis);

                            auto showNotifyThread = std::make_shared<core::JavaThread>(stackFrame);
                            registry.getInterpreter()->execute(showNotifyThread, 10000);
                            goto showNotifyDone;
                        }
                    }
                    currentCls = currentCls->superClass;
                }
                showNotifyDone:;

                g_repaintRequest.requestFull();
                g_lastShownDisplayable = g_currentDisplayable;
            }
        }
    );
}

}
}
