#include"sc_ff_helper.h"

#define TAG "FF_ME"
#include <stdarg.h> //for vsnprintf

#include"sc_ffmpeg_common.h" //for AV_LOG_***
//偶也会改成类的形式来搞
//HiffmpegDecoder::
static void ffmpegNotify(void* ptr, int level, const char* fmt, va_list vl)
{
    char tmpBuffer[1024];
//    __android_log_print(ANDROID_LOG_ERROR, TAG, "AV_LOG_ERROR: %s", tmpBuffer);
      vsnprintf(tmpBuffer,1024,fmt,vl);

    switch(level) {
            /**
             * Something went really wrong and we will crash now.
             */
        case AV_LOG_PANIC:
            MMLOGE(TAG, "AV_LOG_PANIC: %s", tmpBuffer);
            //sPlayer->mCurrentState = MEDIA_PLAYER_STATE_ERROR;
            break;

            /**
             * Something went wrong and recovery is not possible.
             * For example, no header was found for a format which depends
             * on headers or an illegal combination of parameters is used.
             */
        case AV_LOG_FATAL:
            MMLOGE(TAG, "AV_LOG_FATAL: %s", tmpBuffer);
            //sPlayer->mCurrentState = MEDIA_PLAYER_STATE_ERROR;
            break;

            /**
             * Something went wrong and cannot losslessly be recovered.
             * However, not all future data is affected.
             */
        case AV_LOG_ERROR:
            MMLOGE(TAG, "AV_LOG_ERROR: %s", tmpBuffer);
            //sPlayer->mCurrentState = MEDIA_PLAYER_STATE_ERROR;
            break;

            /**
             * Something somehow does not look correct. This may or may not
             * lead to problems. An example would be the use of '-vstrict -2'.
             */
        case AV_LOG_WARNING:
            MMLOGI("AV_LOG_WARNING: %s", tmpBuffer);
            break;

        case AV_LOG_INFO:
            MMLOGI(TAG, "%s", tmpBuffer);
            break;

        case AV_LOG_DEBUG:
            MMLOGI(TAG, "%s", tmpBuffer);
            break;
		default:
			slogw("%s",tmpBuffer);
			break;

    }
}

void create_ffmpeg(){
	av_log_set_callback(ffmpegNotify);
    avcodec_register_all();
	av_register_all();

}

