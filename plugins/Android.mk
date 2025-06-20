LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS += -mthumb -march=armv7 -mfloat-abi=softfp -mfpu=vfp -Os -fvisibility=hidden -fno-stack-protector -D ANDROID
LOCAL_ARM_MODE := thumb

LOCAL_MODULE := opcodes
LOCAL_SRC_FILES := main.cpp
LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)