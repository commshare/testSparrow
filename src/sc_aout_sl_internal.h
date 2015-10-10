#ifndef _SC_SL_AOUT_INTERNAL_H_
#define _SC_SL_AOUT_INTERNAL_H_

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include"sc_lock.h"
#include"sc_buffer.h"

#define 	unlikely(p)   (!!(p))

#define CHECK_OPENSL_ERROR(msg)              \
	if (unlikely(result != SL_RESULT_SUCCESS)) \
	{                                       \
		LOGE("####[%s] error [%s]###",msg,ResultToString(result));\
		goto error; \
	}

// __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
/*
VLCÓÃÁË255°¡
*/
#define OPENSLES_BUFFERS 255 /* maximum number of buffers */
#define OPENSLES_BUFLEN  10   /* ms */


#define SL_LIB_NAME "libOpenSLES.so"
#define SL_ENGINE "slCreateEngine"



typedef SLresult (*slCreateEngine_t)(
        SLObjectItf*, SLuint32, const SLEngineOption*, SLuint32,
        const SLInterfaceID*, const SLboolean*);

#define Destroy(a) (*a)->Destroy(a);
#define SetPlayState(a, b) (*a)->SetPlayState(a, b)
#define RegisterCallback(a, b, c) (*a)->RegisterCallback(a, b, c)
#define GetInterface(a, b, c) (*a)->GetInterface(a, b, c)
#define Realize(a, b) (*a)->Realize(a, b)
#define CreateOutputMix(a, b, c, d, e) (*a)->CreateOutputMix(a, b, c, d, e)
#define CreateAudioPlayer(a, b, c, d, e, f, g) \
    (*a)->CreateAudioPlayer(a, b, c, d, e, f, g)
#define Enqueue(a, b, c) (*a)->Enqueue(a, b, c)
#define Clear(a) (*a)->Clear(a)
#define GetState(a, b) (*a)->GetState(a, b)
#define SetPositionUpdatePeriod(a, b) (*a)->SetPositionUpdatePeriod(a, b)
#define SetVolumeLevel(a, b) (*a)->SetVolumeLevel(a, b)
#define SetMute(a, b) (*a)->SetMute(a, b)


//From vlc
#define CLOCK_FREQ INT64_C(1000000)
#define AOUT_MAX_ADVANCE_TIME           (AOUT_MAX_PREPARE_TIME + CLOCK_FREQ)
#define AOUT_MAX_PREPARE_TIME           (2 * CLOCK_FREQ)
#define AOUT_MIN_PREPARE_TIME           AOUT_MAX_PTS_ADVANCE
#define AOUT_MAX_PTS_ADVANCE            (CLOCK_FREQ / 25)
#define AOUT_MAX_PTS_DELAY              (3 * CLOCK_FREQ / 50)
#define AOUT_MAX_RESAMPLING             10


/*****************************************************************************
 *
 *****************************************************************************/
typedef struct aout_sl
{
    /* OpenSL objects */
    SLObjectItf                     engineObject;
    SLObjectItf                     outputMixObject;
    SLAndroidSimpleBufferQueueItf   playerBufferQueue;
    SLObjectItf                     playerObject;
    SLVolumeItf                     volumeItf;
    SLEngineItf                     engineEngine;
    SLPlayItf                       playerPlay;

    /* OpenSL symbols */
    void                            *p_so_handle;

    slCreateEngine_t                slCreateEnginePtr;
    SLInterfaceID                   SL_IID_ENGINE;
    SLInterfaceID                   SL_IID_ANDROIDSIMPLEBUFFERQUEUE;
    SLInterfaceID                   SL_IID_VOLUME;
    SLInterfaceID                   SL_IID_PLAY;

    /* audio buffered through opensles */
    uint8_t                         *buf;
    int                             samples_per_buf;
    int                             next_buf;

    int                             rate;


    /* if we can measure latency already */
    int                             started;

    int                             samples;
    sc_buf_t                     dbt;
    sc_lock_t lock; //add this 20141210
}aout_sl_t;


#endif
