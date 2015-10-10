#include"sc_audio_out.h"
#include"sc_aout_sl.h"
#include"sc_aout_sl_internal.h"
#include"sc_log.h"
#include <assert.h>
#include <dlfcn.h>
/**
 * Convert OpenSL ES result to string.
 *
 * @param result result code.
 * @return result string.
 */
static const char* ResultToString(SLresult result)
{
	const char* str = 0;

	switch (result)
	{
	case SL_RESULT_SUCCESS:
		str = "Success";
		break;

	case SL_RESULT_PRECONDITIONS_VIOLATED:
		str = "Preconditions violated";
		break;

	case SL_RESULT_PARAMETER_INVALID:
		str = "Parameter invalid";
		break;

	case SL_RESULT_MEMORY_FAILURE:
		str = "Memory failure";
		break;

	case SL_RESULT_RESOURCE_ERROR:
		str = "Resource error";
		break;

	case SL_RESULT_RESOURCE_LOST:
		str = "Resource lost";
		break;

	case SL_RESULT_IO_ERROR:
		str = "IO error";
		break;

	case SL_RESULT_BUFFER_INSUFFICIENT:
		str = "Buffer insufficient";
		break;

	case SL_RESULT_CONTENT_CORRUPTED:
		str = "Success";
		break;

	case SL_RESULT_CONTENT_UNSUPPORTED:
		str = "Content unsupported";
		break;

	case SL_RESULT_CONTENT_NOT_FOUND:
		str = "Content not found";
		break;

	case SL_RESULT_PERMISSION_DENIED:
		str = "Permission denied";
		break;

	case SL_RESULT_FEATURE_UNSUPPORTED:
		str = "Feature unsupported";
		break;

	case SL_RESULT_INTERNAL_ERROR:
		str = "Internal error";
		break;

	case SL_RESULT_UNKNOWN_ERROR:
		str = "Unknown error";
		break;

	case SL_RESULT_OPERATION_ABORTED:
		str = "Operation aborted";
		break;

	case SL_RESULT_CONTROL_LOST:
		str = "Control lost";
		break;

	default:
		str = "Unknown code";
	}

	return str;
}



static SLuint32 convertSampleRate(SLuint32 sr) {
    switch(sr) {
    case 8000:
        return SL_SAMPLINGRATE_8;
    case 11025:
        return SL_SAMPLINGRATE_11_025;
    case 12000:
        return SL_SAMPLINGRATE_12;
    case 16000:
        return SL_SAMPLINGRATE_16;
    case 22050:
        return SL_SAMPLINGRATE_22_05;
    case 24000:
        return SL_SAMPLINGRATE_24;
    case 32000:
        return SL_SAMPLINGRATE_32;
    case 44100:
        return SL_SAMPLINGRATE_44_1;
    case 48000:
        return SL_SAMPLINGRATE_48;
    case 64000:
        return SL_SAMPLINGRATE_64;
    case 88200:
        return SL_SAMPLINGRATE_88_2;
    case 96000:
        return SL_SAMPLINGRATE_96;
    case 192000:
        return SL_SAMPLINGRATE_192;
  }
	LOGE("NOT FOUND SAMPLE RATE[%d]",sr);
  return -1;
}



static inline int bytesPerSample(audio_out_t *aout)
{
    aout_param_t *para = &aout->param;
	//Êý¾ÝÉî¶È£¬Õâ¸öÎÒ»¹Ã»¸³Öµ°¡!
    return para->channels * para->data_width / 8;
    //return 2 /* S16 */ * 2 /* stereo */;
}
static int create_sl_engine(audio_out_t *aout){
	int ret;
	SLresult result;
	if(aout==NULL){
		LOGE("the aout is NULL ");
		ret=-1;
		return ret;
	}
	//dttvÓÃÀ´³õÊ¼»¯Ò»¸ö»º³å
	aout_param_t *param=&aout->param;
	aout_sl_t *sys=NULL;
	sys=(aout_sl_t *)malloc(sizeof(aout_sl_t));
	if(unlikely(sys==NULL))
	{
		LOGE("alloc mem for sys fail");
		return -1;
	}
	slogi("dlopen sl.so");
	sys->p_so_handle=dlopen(SL_LIB_NAME,RTLD_NOW);
	if(sys->p_so_handle == NULL)
	{
		LOGE("dlopen slse lib fail");
		goto error; //Ç°Ãæ·ÖÅäÁËÄÚ´æ¸øsys
	}
	slogi("before create engine ptr");
	sys->slCreateEnginePtr = dlsym(sys->p_so_handle,SL_ENGINE);
	if (unlikely(sys->slCreateEnginePtr == NULL))
    {
		LOGE("GET SL ENGINE FAIL");
        goto error; //ÕâÊ±ºòÓ¦¸Ã×öÉ¶å
    }
	#define OPENSL_DLSYM(dest, name)                       \
    do {                                                       \
        const SLInterfaceID *sym = dlsym(sys->p_so_handle, "SL_IID_"name);        \
        if (unlikely(sym == NULL))                             \
        {                                                      \
            goto error;                                        \
        }                                                      \
        sys->dest = *sym;                                           \
    } while(0)
    OPENSL_DLSYM(SL_IID_ANDROIDSIMPLEBUFFERQUEUE, "ANDROIDSIMPLEBUFFERQUEUE");
    OPENSL_DLSYM(SL_IID_ENGINE, "ENGINE");
    OPENSL_DLSYM(SL_IID_PLAY, "PLAY");
    OPENSL_DLSYM(SL_IID_VOLUME, "VOLUME");
#undef OPENSL_DLSYM
//	slogi("-------1-----");
 		 // create engine
    result = sys->slCreateEnginePtr(&sys->engineObject, 0, NULL, 0, NULL, NULL);
    CHECK_OPENSL_ERROR("Failed to create engine");
	// realize the engine in synchronous mode
	result = Realize(sys->engineObject,SL_BOOLEAN_FALSE);
	CHECK_OPENSL_ERROR("Failed to realize engine");
//	slogi("-------2-----");

	 // get the engine interface, needed to create other objects
	result=GetInterface(sys->engineObject,sys->SL_IID_ENGINE, &sys->engineEngine);
    CHECK_OPENSL_ERROR("Failed to get the engine interface");
//	slogi("-------3-----");

	// create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids1[] = { sys->SL_IID_VOLUME };
    const SLboolean req1[] = { SL_BOOLEAN_FALSE };
    result = CreateOutputMix(sys->engineEngine, &sys->outputMixObject, 1, ids1, req1);
    CHECK_OPENSL_ERROR("Failed to create output mix");
	//slogi("-------4-----");

    // realize the output mix in synchronous mode
    result = Realize(sys->outputMixObject, SL_BOOLEAN_FALSE);
    CHECK_OPENSL_ERROR("Failed to realize output mix");
	//slogi("-------44-----");

	//³õÊ¼»¯
	sc_lock_init(&sys->lock,NULL);
	//Õâ¶¼ÊÇÉ¶ÒâË¼?
	if(buf_init(&sys->dbt,param->sample_rate*4/10) < 0) //100ms ???
	{
		LOGE("#######################ERROR");
		return -1;
	}
	//only sucess ,we will do this
	aout->priv=(void *)sys;
	//	slogi("-------5-----");

	return 0;

	error:
		#if 1 //Ò»µ©³ö´í£¬ÈË¼Ò×Ô¼º»á´¦ÀíÕâÐ©
		if (sys->outputMixObject)
        Destroy(sys->outputMixObject);
	    if (sys->engineObject)
    	    Destroy(sys->engineObject);
	    if (sys->p_so_handle)
    	    dlclose(sys->p_so_handle);
		free(sys);
		#endif
	return -1;
}
int uninit_engine(audio_out_t *aout){
		//Ç°ÌáÊÇcreate engine ³É¹¦¹ý! ²ÅÄÜÄÃµ½Õâ¸ö
		aout_sl_t *sys=(aout_sl_t *)aout->priv;

		if (sys->outputMixObject)
        Destroy(sys->outputMixObject);
	    if (sys->engineObject)
    	    Destroy(sys->engineObject);
	    if (sys->p_so_handle)
    	    dlclose(sys->p_so_handle);
		free(sys);
		return 0;
}
static void PlayedCallback (SLAndroidSimpleBufferQueueItf caller, void *pContext)
{
    (void)caller;
    audio_out_t *aout = pContext;
    aout_sl_t *sys = (aout_sl_t *)aout->priv;

    assert (caller == sys->playerBufferQueue);
    sys->started = 1;
    //__android_log_print(ANDROID_LOG_DEBUG,TAG, "opensl callback called \n");

}
static int create_sl_player(audio_out_t *aout){
	SLresult result;
	aout_sl_t *sys=(aout_sl_t *)aout->priv;
	aout_param_t *param=&aout->param;
	// configure audio source - this defines the number of samples you can enqueue.
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {
        SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
        OPENSLES_BUFFERS
    };
	int mask;
	if(param->channels >1)
		mask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    else
        mask = SL_SPEAKER_FRONT_CENTER;
	//__FUNCTION__,//
	slogi("%s begin: numChannels[%d]  sample_rate[%d]",__FUNCTION__,param->channels,param->sample_rate);
	SLDataFormat_PCM format_pcm;
	format_pcm.formatType       = SL_DATAFORMAT_PCM;
    format_pcm.numChannels      = param->channels;
    //format_pcm.samplesPerSec    = ((SLuint32) para->dst_samplerate * 1000) ;
    format_pcm.samplesPerSec    = ((SLuint32) convertSampleRate(param->sample_rate)) ;
    format_pcm.bitsPerSample    = SL_PCMSAMPLEFORMAT_FIXED_16;
    format_pcm.containerSize    = SL_PCMSAMPLEFORMAT_FIXED_16;
    format_pcm.channelMask      = mask;
    format_pcm.endianness       = SL_BYTEORDER_LITTLEENDIAN;
	slogi("%s end: numChannels[%d]  samplesPerSec[%d]",__FUNCTION__,format_pcm.numChannels,format_pcm.samplesPerSec);

	SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {
        SL_DATALOCATOR_OUTPUTMIX,
        sys->outputMixObject
    };
    SLDataSink audioSnk = {&loc_outmix, NULL};
	//create audio player
    const SLInterfaceID ids2[] = { sys->SL_IID_ANDROIDSIMPLEBUFFERQUEUE, sys->SL_IID_VOLUME };
    static const SLboolean req2[] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
    result = CreateAudioPlayer(sys->engineEngine, &sys->playerObject, &audioSrc,
                                    &audioSnk, sizeof(ids2) / sizeof(*ids2),
                                    ids2, req2);
    if (unlikely(result != SL_RESULT_SUCCESS)) { // error
    LOGE("aout_sl CreateAudioPlayer  fail !!!");
        return -1;
    }
        /* Try again with a more sensible samplerate */

	    CHECK_OPENSL_ERROR("Failed to create audio player");

    result = Realize(sys->playerObject, SL_BOOLEAN_FALSE);
    CHECK_OPENSL_ERROR("Failed to realize player object.");

    result = GetInterface(sys->playerObject, sys->SL_IID_PLAY, &sys->playerPlay);
    CHECK_OPENSL_ERROR("Failed to get player interface.");

    result = GetInterface(sys->playerObject, sys->SL_IID_VOLUME, &sys->volumeItf);
    CHECK_OPENSL_ERROR("failed to get volume interface.");

    result = GetInterface(sys->playerObject, sys->SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                                  &sys->playerBufferQueue);
    CHECK_OPENSL_ERROR("Failed to get buff queue interface");

    result = RegisterCallback(sys->playerBufferQueue, PlayedCallback,
                                   (void*)aout);
    CHECK_OPENSL_ERROR("Failed to register buff queue callback.");

    // set the player's state to playing
    result = SetPlayState(sys->playerPlay, SL_PLAYSTATE_PLAYING);
    CHECK_OPENSL_ERROR("Failed to switch to playing state");

	sys->rate=param->sample_rate;
	//Õâ¸öÊÇå Ã¿¸ö»º³åÓÐ¶àÉÙ¸ö²ÉÑùµã?
	sys->samples_per_buf=OPENSLES_BUFLEN * param->sample_rate / 1000;

	//ÔÚÕâÀï³õÊ¼»¯°¡
	sys->buf=malloc(OPENSLES_BUFFERS * sys->samples_per_buf * bytesPerSample(aout));
    if (!sys->buf)
        goto error;
    sys->started = 0;
    sys->next_buf = 0;
    sys->samples = 0;
	//Õâ¸öÊÇ¸ÉÉ¶µÄ£¬²»¶®°¡
    SetPositionUpdatePeriod(sys->playerPlay, AOUT_MIN_PREPARE_TIME * 1000 / CLOCK_FREQ);
	return 0;
error:
	if (sys->playerObject) {
        Destroy(sys->playerObject);
        sys->playerObject = NULL;
    }

    return -1;

}
	//static
 static void stop_sl_player(audio_out_t *aout)
{
    aout_sl_t *sys = (aout_sl_t *)aout->priv;

    SetPlayState(sys->playerPlay, SL_PLAYSTATE_STOPPED);
    //Flush remaining buffers if any.
    Clear(sys->playerBufferQueue);

    free(sys->buf);

    Destroy(sys->playerObject);
    sys->playerObject = NULL;
    free(sys);
    sys = NULL;
}

//export
int aout_sl_stop(audio_out_t *aout){
    aout_sl_t *sys = (aout_sl_t*)aout->priv;
	sc_lock(&sys->lock);
	stop_sl_player(aout);
	sc_unlock(&sys->lock);
	return 0;
}
/*****************************************************************************
 *
 *****************************************************************************/
 #if 0
 void Close(audio_out_t *aout)
{
    aout_sl_t *sys = (aout_sys_t *)aout->ao_priv;

    Destroy(sys->outputMixObject);
    Destroy(sys->engineObject);
    dlclose(sys->p_so_handle);
    vlc_mutex_destroy(&sys->lock);
    free(sys);
}
 #endif

static int WriteBuffer(audio_out_t *aout)
{
    aout_sl_t *sys = (aout_sl_t *)aout->priv;
    const int unit_size = sys->samples_per_buf * bytesPerSample(aout);

    /* Check if we can fill at least one buffer unit by chaining blocks */
    if (sys->dbt.level  <  unit_size) {
        return -1;
    }

    SLAndroidSimpleBufferQueueState st;
    SLresult res = GetState(sys->playerBufferQueue, &st);
    if (unlikely(res != SL_RESULT_SUCCESS)) {
        return -1;
    }

    if (st.count == OPENSLES_BUFFERS)
        return -1;

    int done = 0;
    while (done < unit_size) {
        int cur = buf_level(&sys->dbt);
        if (cur > unit_size - done)
            cur = unit_size - done;

        //memcpy(&sys->buf[unit_size * sys->next_buf + done], b->p_buffer, cur);
        buf_get(&sys->dbt,&sys->buf[unit_size * sys->next_buf + done],cur);
        done += cur;

        if (done == unit_size)
            break;
    }

    SLresult r = Enqueue(sys->playerBufferQueue,
        &sys->buf[unit_size * sys->next_buf], unit_size);

    sys->samples -= sys->samples_per_buf;
    //__android_log_print(ANDROID_LOG_DEBUG,TAG, "minus sampels, %d minus %d \n",sys->samples, sys->samples_per_buf);

    if (r == SL_RESULT_SUCCESS) {
        if (++sys->next_buf == OPENSLES_BUFFERS)
            sys->next_buf = 0;
        return 0;
    } else {
        /* XXX : if writing fails, we don't retry */
        return -1;
    }
}

/*****************************************************************************
 * Play: play a sound
 *****************************************************************************/

 static int Play(audio_out_t *aout, uint8_t *buf, int size)
{
    aout_sl_t *sys = (aout_sl_t *)aout->priv;
    int ret = 0;
    //__android_log_print(ANDROID_LOG_DEBUG,TAG, "space:%d level:%d  size:%d  \n",buf_space(&sys->dbt), buf_level(&sys->dbt), size);
    //Ô­À´aoutÀïÍ·È¡³öÀ´µÄÊý¾ÝÒ²ÊÇÒªÏÈ¶ªÈëÕâÀï»º³åÏÂµÄ°¡!!!!!
    if(buf_space(&sys->dbt) > size)
    {
        ret = buf_put(&sys->dbt,buf,size);
    }
    sys->samples += ret / bytesPerSample(aout);
    //__android_log_print(ANDROID_LOG_DEBUG,TAG, "add sampels, %d add %d \n",sys->samples, ret / bytesPerSample(aout));

    /* Fill OpenSL buffer */
    WriteBuffer(aout); // will read data in callback
    return ret;
}
int aout_sl_write (audio_out_t *aout, uint8_t * buf, int size)
{
    aout_sl_t *sys = (aout_sl_t*)aout->priv;
    int ret = 0;
    sc_lock(&sys->lock);
    ret = Play(aout,buf,size);
    sc_unlock(&sys->lock);
    return ret;
}

int aout_sl_init(audio_out_t *aout){
	int ret;
		//sl
	slogi("before create_sl_engine");
	ret=create_sl_engine(aout);
	if(ret == -1){
		LOGE("create sl engine fail");
		goto ERR_CREATE_SL_ENEGINE;
	}
	slogi("before create_sl_player");
	ret=create_sl_player(aout);
	if(ret == -1){
		LOGE("create sl player fail");
		goto ERR_CREATE_SL_PLAYER;
	}
	//all ok
	return 0;

	ERR_CREATE_SL_ENEGINE:
		//uninit_engine(aout);
	ERR_CREATE_SL_PLAYER:
	//	uninit_player(aout);

	   uninit_engine(aout);
		return ret;
}

