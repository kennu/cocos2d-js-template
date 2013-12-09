LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := game_shared

LOCAL_MODULE_FILENAME := libgame

LOCAL_SRC_FILES := main/main.cpp \
                   ../../Classes/AppDelegate.cpp \
                   ../../Classes/JSB_AUTO.cpp
                   
LOCAL_C_INCLUDES := $(LOCAL_PATH)/main \
                    $(LOCAL_PATH)/../../Classes

LOCAL_WHOLE_STATIC_LIBRARIES := cocos2dx_static cocosdenshion_static cocos_extension_static
LOCAL_WHOLE_STATIC_LIBRARIES += scriptingcore-spidermonkey
            
LOCAL_EXPORT_FLAGS := -DCOCOS2D_JAVASCRIPT

include $(BUILD_SHARED_LIBRARY)

$(call import-module,CocosDenshion/android) \
$(call import-module,cocos2dx) \
$(call import-module,extensions)
$(call import-module,scripting/javascript/spidermonkey-android)
$(call import-module,scripting/javascript/bindings)
