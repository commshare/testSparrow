# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
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
ME_FF_NAME=dep/libmeff.so

LOCAL_C_INCLUDES +=$(ME_FF_DIR)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include

#jniSL
LOCAL_MODULE    := ffbitmap
LOCAL_SRC_FILES := native.c

#com_yanlong_opensl_VideoPlayer.c \
ff_audio_dec.c \
sc_aout_sl.c\
sc_audio_decoder.c \
sc_audio_out.c \
sc_buffer.c \
sc_demuxer.c \
sc_queue.c \
sc_ff_helper.c
 

LOCAL_LDLIBS= -L$(LOCAL_PATH)/../dep/ -lmeff

# for native audio
#LOCAL_LDLIBS    += -lOpenSLES
# for logging
LOCAL_LDLIBS    += -llog
# for native asset manager
LOCAL_LDLIBS    += -landroid -ljnigraphics

#LOCAL_STATIC_LIBRARIES := 

#LOCAL_SHARED_LIBRARIES := $(ME_FF_NAME)

include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_EXECUTABLE)
