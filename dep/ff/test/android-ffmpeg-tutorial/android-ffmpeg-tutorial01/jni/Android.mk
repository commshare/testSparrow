LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

DT_FF_OUT=
#/home/zhangbin/zbhome/videoplayer/ffmpeg2/ffplay/ijkplayer/ijkplayer-android-master/android/build/ffmpeg-armv7a/output

###use my 2.8
FF_HOME=/home/zhangbin/nuf/sc/sparrow/dep/ff
#/home/zhangbin/zbhome/dttv201409/sl/ff
FF_SRC=$(FF_HOME)/ffmpeg-2.8
PREFIX=$(FF_HOME)/build/ffmpeg/armv7

ME_FF_DIR=$(PREFIX)
###libffmepg.so
#ME_FF_NAME=
MEFF=/home/zhangbin/nuf/sc/sparrow/dep/ff/test/bitmap/dep

LOCAL_C_INCLUDES +=$(ME_FF_DIR)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include

LOCAL_LDLIBS := -L$(MEFF) -lmeff


LOCAL_MODULE    := tutorial01
LOCAL_SRC_FILES := tutorial01.c
LOCAL_LDLIBS += -llog -ljnigraphics -lz 
#LOCAL_SHARED_LIBRARIES := libavformat libavcodec libswscale libavutil

include $(BUILD_SHARED_LIBRARY)
#$(call import-module,ffmpeg-2.0.1/android/arm)
