#ifndef _SC_NO_USED_H_
#define _SC_NO_USED_H_




struct openslgs_info_t{
	// engine interfaces
	 SLObjectItf 					engineObject ;
	 SLEngineItf 					engineEngine;


	// output mix interfaces
	 SLObjectItf 					outputMixObject ;
	 SLEnvironmentalReverbItf 		outputMixEnvironmentalReverb ;

	// buffer queue player interfaces
	 SLObjectItf 					bqPlayerObject ;
	 SLPlayItf 						bqPlayerPlay;
	 SLAndroidSimpleBufferQueueItf 	bqPlayerBufferQueue;
	 SLEffectSendItf 				bqPlayerEffectSend;
	 SLMuteSoloItf 					bqPlayerMuteSolo;
	 SLVolumeItf 					bqPlayerVolume;

}openslgs;










int quit = 0;




AVPacket packet;
AVFrame *frame;
#if 0
int decode_frame(uint8_t *stream){
	  int len, data_size, got_frame;
	for(;;){
		while(packet.size>0){
			 if(!frame)
			  {
			      if (!(frame = avcodec_alloc_frame()))
			        return AVERROR(ENOMEM);
			   }
			   else
			   {
			       avcodec_get_frame_defaults(frame);
			    }
			  len = avcodec_decode_audio4(ffmpeg_info.dec_ctx, frame, &got_frame, &packet);

              if(len < 0)
              {
                  /* if error, skip frame */
            	  packet.size = 0;
                  break;
              }
              packet.data += len;
              packet.size -= len;

              if(got_frame <= 0) /* No data yet, get more frames */
                  continue;
              data_size = av_samples_get_buffer_size(NULL, ffmpeg_info.dec_ctx->channels, frame->nb_samples, ffmpeg_info.dec_ctx->sample_fmt, 1);
              memcpy(stream, frame->data[0], data_size);

             return data_size;
     //         return 0;
		}
		packet_queue_get(&audio_queue,&packet,1);

	}
}
#endif





void createEngine(){



	  SLresult result;
	  // create engine
	  result = slCreateEngine(&openslgs.engineObject, 0, NULL, 0, NULL, NULL);
	  assert(SL_RESULT_SUCCESS == result);
	  // realize the engine
	  result = (*openslgs.engineObject)->Realize(openslgs.engineObject, SL_BOOLEAN_FALSE);
	  assert(SL_RESULT_SUCCESS == result);
	  // get the engine interface
	  result = (*openslgs.engineObject)->GetInterface(openslgs.engineObject, SL_IID_ENGINE, &openslgs.engineEngine);
	  assert(SL_RESULT_SUCCESS == result);
	  // create output mix
	  const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
	  const SLboolean req[1] = {SL_BOOLEAN_FALSE};
	  result = (*openslgs.engineEngine)->CreateOutputMix(openslgs.engineEngine, &openslgs.outputMixObject, 1, ids, req);
	  assert(SL_RESULT_SUCCESS == result);
	  // realize the output mix
	  result = (*openslgs.outputMixObject)->Realize(openslgs.outputMixObject, SL_BOOLEAN_FALSE);
	  assert(SL_RESULT_SUCCESS == result);
	#if 0
	  // get the environmental reverb interface
	  result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
	        &outputMixEnvironmentalReverb);
	  if (SL_RESULT_SUCCESS == result) {
	    result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(outputMixEnvironmentalReverb, &reverbSettings);
	  }
	#endif
	  // ignore unsuccessful result codes for env reverb
}
#if 0
void createBufferQueueAudioPlayer(){
    SLresult result;

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};


/*    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_16,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
    format_pcm.samplesPerSec=ffmpeg_info.dec_ctx->sample_rate*1000;

      SLDataSource audioSrc = {&loc_bufq, &format_pcm};
    */

    SLDataFormat_PCM pcm;
    pcm.formatType = SL_DATAFORMAT_PCM;
    pcm.numChannels = ffmpeg_info.dec_ctx->channels;//跟下面的channelMask 要配对 不会会报错

    pcm.samplesPerSec = ffmpeg_info.dec_ctx->sample_rate*1000;
    pcm.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
    pcm.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
    if(pcm.numChannels==2)
    	pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;  //立体声 这是参照audiotrack CHANNEL_OUT_STEREO = (CHANNEL_OUT_FRONT_LEFT | CHANNEL_OUT_FRONT_RIGHT)得到的
    else
    	pcm.channelMask=SL_SPEAKER_FRONT_CENTER;
    pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;

    SLDataSource audioSrc = {&loc_bufq, &pcm};



    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, openslgs.outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // create audio player
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND,
            /*SL_IID_MUTESOLO,*/ SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            /*SL_BOOLEAN_TRUE,*/ SL_BOOLEAN_TRUE};
    result = (*openslgs.engineEngine)->CreateAudioPlayer(openslgs.engineEngine, &openslgs.bqPlayerObject, &audioSrc, &audioSnk,
            3, ids, req);
    assert(SL_RESULT_SUCCESS == result);

    // realize the player
    result = (*openslgs.bqPlayerObject)->Realize(openslgs.bqPlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    // get the play interface
    result = (*openslgs.bqPlayerObject)->GetInterface(openslgs.bqPlayerObject, SL_IID_PLAY, &openslgs.bqPlayerPlay);
    assert(SL_RESULT_SUCCESS == result);

    // get the buffer queue interface
    result = (*openslgs.bqPlayerObject)->GetInterface(openslgs.bqPlayerObject, SL_IID_BUFFERQUEUE,
            &openslgs.bqPlayerBufferQueue);
    assert(SL_RESULT_SUCCESS == result);

    // register callback on the buffer queue
    result = (*openslgs.bqPlayerBufferQueue)->RegisterCallback(openslgs.bqPlayerBufferQueue, bqPlayerCallback, NULL);
    assert(SL_RESULT_SUCCESS == result);


    assert(SL_RESULT_SUCCESS == result);

#if 0   // mute/solo is not supported for sources that are known to be mono, as this is
    // get the mute/solo interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_MUTESOLO, &bqPlayerMuteSolo);
    assert(SL_RESULT_SUCCESS == result);
#endif

    // get the volume interface
    result = (*openslgs.bqPlayerObject)->GetInterface(openslgs.bqPlayerObject, SL_IID_VOLUME, &openslgs.bqPlayerVolume);
    assert(SL_RESULT_SUCCESS == result);

    // set the player's state to playing
    result = (*openslgs.bqPlayerPlay)->SetPlayState(openslgs.bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);
}
#endif



void ylcreateBufferQueueAudioPlayer(){
    SLresult result;

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};


/*    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_16,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
    format_pcm.samplesPerSec=ffmpeg_info.dec_ctx->sample_rate*1000;

      SLDataSource audioSrc = {&loc_bufq, &format_pcm};
    */

    SLDataFormat_PCM pcm;
    pcm.formatType = SL_DATAFORMAT_PCM;
    pcm.numChannels = ffmpeg_info.dec_ctx->channels;//跟下面的channelMask 要配对 不会会报错

    pcm.samplesPerSec = ffmpeg_info.dec_ctx->sample_rate*1000;
    pcm.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
    pcm.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
    if(pcm.numChannels==2)
    	pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;  //立体声 这是参照audiotrack CHANNEL_OUT_STEREO = (CHANNEL_OUT_FRONT_LEFT | CHANNEL_OUT_FRONT_RIGHT)得到的
    else
    	pcm.channelMask=SL_SPEAKER_FRONT_CENTER;
    pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;

    SLDataSource audioSrc = {&loc_bufq, &pcm};



    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, openslgs.outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // create audio player
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND,
            /*SL_IID_MUTESOLO,*/ SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            /*SL_BOOLEAN_TRUE,*/ SL_BOOLEAN_TRUE};
    result = (*openslgs.engineEngine)->CreateAudioPlayer(openslgs.engineEngine, &openslgs.bqPlayerObject, &audioSrc, &audioSnk,
            3, ids, req);
    assert(SL_RESULT_SUCCESS == result);

    // realize the player
    result = (*openslgs.bqPlayerObject)->Realize(openslgs.bqPlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    // get the play interface
    result = (*openslgs.bqPlayerObject)->GetInterface(openslgs.bqPlayerObject, SL_IID_PLAY, &openslgs.bqPlayerPlay);
    assert(SL_RESULT_SUCCESS == result);

    // get the buffer queue interface
    result = (*openslgs.bqPlayerObject)->GetInterface(openslgs.bqPlayerObject, SL_IID_BUFFERQUEUE,
            &openslgs.bqPlayerBufferQueue);
    assert(SL_RESULT_SUCCESS == result);

    // register callback on the buffer queue
    result = (*openslgs.bqPlayerBufferQueue)->RegisterCallback(openslgs.bqPlayerBufferQueue, bqPlayerCallback, NULL);
    assert(SL_RESULT_SUCCESS == result);


    assert(SL_RESULT_SUCCESS == result);

#if 0   // mute/solo is not supported for sources that are known to be mono, as this is
    // get the mute/solo interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_MUTESOLO, &bqPlayerMuteSolo);
    assert(SL_RESULT_SUCCESS == result);
#endif

    // get the volume interface
    result = (*openslgs.bqPlayerObject)->GetInterface(openslgs.bqPlayerObject, SL_IID_VOLUME, &openslgs.bqPlayerVolume);
    assert(SL_RESULT_SUCCESS == result);

    // set the player's state to playing
    result = (*openslgs.bqPlayerPlay)->SetPlayState(openslgs.bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);
    LOGI("create player ok");

}

void createBufferQueueAudioPlayer(){
    SLresult result;

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq =
    {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, OPENSLES_BUFFERS/*buffer count*/};


/*    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_16,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
    format_pcm.samplesPerSec=ffmpeg_info.dec_ctx->sample_rate*1000;

      SLDataSource audioSrc = {&loc_bufq, &format_pcm};
    */
/*
	//搞一些信息出来
	dtaudio_para_t mAudioParam;
	memset(&mAudioParam,0,sizeof(dtaudio_para_t));
	AVCodecContext *pAudioCtx=ffmpeg_info.dec_ctx.;
	//audio sample format
	mAudioParam.afmt=pAudioCtx->sample_fmt;
	mAudioParam.channels=pAudioCtx->channels;
	mAudioParam.samplerate=pAudioCtx->sample_fmt;
*/
    SLDataFormat_PCM pcm;
    pcm.formatType = SL_DATAFORMAT_PCM;
    pcm.numChannels = ffmpeg_info.dec_ctx->channels;//跟下面的channelMask 要配对 不会会报错
	LOGE("PCM channels num [%d]  samplesPerSec[%d]",ffmpeg_info.dec_ctx->channels,ffmpeg_info.dec_ctx->sample_rate);
    pcm.samplesPerSec = ((SLuint32) convertSampleRate(ffmpeg_info.dec_ctx->sample_rate));//ffmpeg_info.dec_ctx->sample_rate*1000;
    pcm.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
    pcm.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
	if(pcm.numChannels>1)
	    pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;  //立体声 这是参照audiotrack CHANNEL_OUT_STEREO = (CHANNEL_OUT_FRONT_LEFT | CHANNEL_OUT_FRONT_RIGHT)得到的
	else
		pcm.channelMask =SL_SPEAKER_FRONT_CENTER;
    pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;

    SLDataSource audioSrc = {&loc_bufq, &pcm};



    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, openslgs.outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};
    //////////////////////////下面的开始不一样了。///////////
    // create audio player
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND,
            /*SL_IID_MUTESOLO,*/ SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            /*SL_BOOLEAN_TRUE,*/ SL_BOOLEAN_TRUE};

    //vlc-----create audio player
    const SLInterfaceID ids2[] = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_VOLUME };
    static const SLboolean req2[] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };

	//果然是创建播放器错误了!主要还是参数错误!
    result = (*openslgs.engineEngine)->CreateAudioPlayer(openslgs.engineEngine, &openslgs.bqPlayerObject, &audioSrc, &audioSnk,
            sizeof(ids2) / sizeof(*ids2), ids2,req2);//3,ids, req);
	CHECK_OPENSL_ERROR(result);
    assert(SL_RESULT_SUCCESS == result);

    // realize the player
    result = (*openslgs.bqPlayerObject)->Realize(openslgs.bqPlayerObject, SL_BOOLEAN_FALSE);
	CHECK_OPENSL_ERROR(result);
    assert(SL_RESULT_SUCCESS == result);

    // get the play interface
    result = (*openslgs.bqPlayerObject)->GetInterface(openslgs.bqPlayerObject, SL_IID_PLAY, &openslgs.bqPlayerPlay);
	CHECK_OPENSL_ERROR(result);
    assert(SL_RESULT_SUCCESS == result);

    // get the buffer queue interface
    result = (*openslgs.bqPlayerObject)->GetInterface(openslgs.bqPlayerObject, SL_IID_BUFFERQUEUE,
            &openslgs.bqPlayerBufferQueue);
	CHECK_OPENSL_ERROR(result);
    assert(SL_RESULT_SUCCESS == result);

    // register callback on the buffer queue
    result = (*openslgs.bqPlayerBufferQueue)->RegisterCallback(openslgs.bqPlayerBufferQueue, bqPlayerCallback, NULL);
	CHECK_OPENSL_ERROR(result);
    assert(SL_RESULT_SUCCESS == result);


 //   assert(SL_RESULT_SUCCESS == result);

#if 0   // mute/solo is not supported for sources that are known to be mono, as this is
    // get the mute/solo interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_MUTESOLO, &bqPlayerMuteSolo);
    assert(SL_RESULT_SUCCESS == result);
#endif

    // get the volume interface
    result = (*openslgs.bqPlayerObject)->GetInterface(openslgs.bqPlayerObject, SL_IID_VOLUME, &openslgs.bqPlayerVolume);
    CHECK_OPENSL_ERROR(result);
    assert(SL_RESULT_SUCCESS == result);

    // set the player's state to playing
    result = (*openslgs.bqPlayerPlay)->SetPlayState(openslgs.bqPlayerPlay, SL_PLAYSTATE_PLAYING);
	CHECK_OPENSL_ERROR(result);
    assert(SL_RESULT_SUCCESS == result);
	LOGE("===create buffer queue playre ok ===");
}

#endif

