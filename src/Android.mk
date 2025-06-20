LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#NDK_DEBUG := 1
#APP_OPTIM := debug

LOCAL_CFLAGS += -mthumb -march=armv7 -mfloat-abi=softfp -mfpu=vfp -Os -fvisibility=hidden -fno-stack-protector -D ANDROID #-std=c++11 #-S
LOCAL_ARM_MODE := thumb

LOCAL_MODULE    := cleo
LOCAL_SRC_FILES := main.cpp core.cpp armhook.cpp utils.cpp text.cpp touch.cpp libres.cpp ui.cpp plugins.cpp pattern.cpp mutex.cpp strutils.cpp memutils.cpp
LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)