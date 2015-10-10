#ifndef _SC_LOG_H_
#define _SC_LOG_H_

#include <android/log.h>

#define  LOG_TAG    "SC_ME"
#define FF_TMP "FF_"
#if 1
//#define  LOGI(...)
#define LOGI(message,...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "[%s_%d_%s]"message,__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define LOGD(message,...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "[%s_%d_%s]"message,__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define LOGE(message,...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,"[%s_%d_%s]"message,__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)

#define slogi(message,...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG,"##"message,##__VA_ARGS__)
#define slogd(message,...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG,"##"message,##__VA_ARGS__)
#define sloge(message,...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,"##"message,##__VA_ARGS__)
#define slogw(message,...) __android_log_print(ANDROID_LOG_WARN, FF_TMP,"##"message,##__VA_ARGS__)


#define  MMLOGI(TAG, ...)   __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define  MMLOGD(TAG, ...)   __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define  MMLOGE(TAG, ...)   __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define  MMLOGW(TAG, ...)   __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define  MMLOGV(TAG, ...)   __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

#else
#define LOGI(format,...)  printf("file[%s] line[%d] (function %s)"format"\n",__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define LOGD(format,...)  printf("file[%s] line[%d] (function %s) "format"\n",__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define LOGE(format,...)  printf("file[%s] line[%d] (function %s) "format"\n",__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__)
#endif


#endif
