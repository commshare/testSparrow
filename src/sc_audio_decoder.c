#include"sc_audio_decoder.h"


#include"sc_decoder_internal.h"
#include"ff_audio_dec_internal.h"
#include"sc_queue.h"


static int audio_decoder_read_frame(PacketQueue *pPktQueue,AVPacket *frame )
{

	//ÉèÖÃÎª1£¬ÕâÑùÄÃ²»µ½»á·µ»ØÒ»¸öretÎª -1
	 slogi("before packet_queue_get");
	 int ret=packet_queue_get(pPktQueue,frame,1);
	#if 1
	if(ret == -1)
	{
		LOGE("can not get raw frame from audio demuxer out queue");
	}
	#endif
	return ret;

}
static int64_t pts_exchange (audio_decoder_t * decoder, int64_t pts)
{
    return pts;
}

static void *audio_decode_loop (void *arg)
{
    int ret;
    audio_decoder_t *decoder = (audio_decoder_t *) arg;
    audio_param_t *para = &decoder->param;
	//·ÇÒªÓÃ×Ô¼ºµÄ¸ñÊ½¡£
   // dt_av_pkt_t frame;
     AVPacket frame;
//    ad_wrapper_t *wrapper = decoder->wrapper;
 //   dtaudio_context_t *actx = (dtaudio_context_t *) decoder->parent;
    sc_buf_t *out = &decoder->decoder_out_buf;//ÄÄÀï·ÖÅäÄÚ´æºÍ³õÊ¼»¯ÄØ?

    int declen, fill_size;

    //for some type audio, can not read completly frame
    uint8_t *frame_data = NULL; // point to frame start
    uint8_t *rest_data = NULL;
    int frame_size = 0;
    int rest_size = 0;

    int used;                   // used size after every decode ops

    adec_ctrl_t *pinfo = &decoder->info;
    memset(pinfo,0,sizeof(*pinfo));
    pinfo->channels = para->src_channels;// para->channels;
    pinfo->samplerate =para->src_samplerate;// para->samplerate;
    //Ô­À´ÊÇÔÚÕâÀï·ÖÅäµÄÄÚ´æ°¡,´æÒ»Ö¡ÒôÆµ
    pinfo->outptr = malloc(MAX_ONE_FRAME_OUT_SIZE); //192000
    pinfo->outsize = MAX_ONE_FRAME_OUT_SIZE;

   // ffmpeg_info_t *ff=(ffmpeg_info_t *)decoder->parent;
    PacketQueue *pPktQueue=decoder->adec_in_queue;
	if(pPktQueue==NULL){
		ret=-1;
		LOGE("cannot get packetqueue ptr");
		//void µÄ·µ»ØÖµ!!!!!
		return;
	}

  	slogi("==============[%s:%d] AUDIO DECODE START ", __FUNCTION__, __LINE__);
    do
    {
		#if 0
        //maybe receive exit cmd in idle status, so exit prior to idle
        if (decoder->status == ADEC_STATUS_EXIT)
        {
            LOGD("[%s:%d] receive decode loop exit cmd \n", __FUNCTION__, __LINE__);
            if (frame_data)
                free (frame_data);
            if (rest_data)
                free (rest_data);
            break;
        }

        if (decoder->status == ADEC_STATUS_IDLE)
        {
             LOGD("[%s:%d] Idle status ,please wait \n", __FUNCTION__, __LINE__);
            usleep (100);
            continue;
        }
		#endif
        /*read frame */
		#if 0
        if (!decoder->parent)  //ÓÐ¿ÉÄÜ»¹Ã»±»´´½¨??
        {
            usleep (10000);
            dt_info (TAG, "[%s:%d] decoder parent is NULL \n", __FUNCTION__, __LINE__);
            continue;
        }
		#endif
		//Òª´ÓdemuxerµÄÊä³ö»º³åÀïÍ·¶ÁÈ¡Êý¾ÝµÄ
		//slogi("before read");
        ret = audio_decoder_read_frame (pPktQueue, &frame);
		slogi("after audio_decoder_read_frame ret[%d]",ret);
        if (ret < 0 || frame.size <= 0)
        {
			LOGE("sleep 1000");
			//Õâ¾ÍÊÇÃ»ÓÐ¶ÁÈ¡µ½°¡
            usleep (1000);
          //  dt_debug (TAG, "[%s:%d] dtaudio decoder loop read frame failed \n", __FUNCTION__, __LINE__);
            continue;
        }
        //read ok,update current pts, clear the buffer size
        if (frame.pts >= 0)
        {
            if (decoder->pts_first == -1)//É¶Ê±ºò±»³õÊ¼»¯Îª-1µÄå
            {
                if(frame.pts == SC_NOPTS_VALUE)
                    frame.pts = 0;
                decoder->pts_first = pts_exchange (decoder, frame.pts);
                LOGI("first frame pts:%lld dts:%lld duration:%d size:%d\n", decoder->pts_first, frame.dts, frame.duration, frame.size);
            }
            decoder->pts_current = pts_exchange (decoder, frame.pts);
            LOGD("pkt pts:%lld current:%lld duration:%d pts_s:%lld dts:%lld buf_size:%d \n", frame.pts, decoder->pts_current, frame.duration, frame.pts/* / 90000*/, frame.dts, decoder->pts_buffer_size);
            decoder->pts_last_valid = decoder->pts_current;
            decoder->pts_buffer_size = 0; //Õâ¸öÊÇ¸ÉÉ¶ÓÃµÄ?
        }
        //repack the frame
        if (frame_data)//ÕâÊÇ°ÑÉÏÒ»´ÎµÄÊý¾ÝÖ¸ÕëÇå¿Õ
        {
            free (frame_data);
            frame_data = NULL;
            frame_size = 0;
        }
		//ÕâÊÇÉÏÒ»´ÎÃ»ÓÃÍæµÄÊý¾Ý£¬Í³Ò»ÓÃµ½Õâ´Î
        if (rest_size)
            frame_data = malloc (frame.size + rest_size);
        else
            frame_data = frame.data; //Ö±½ÓÖ¸ÏòpktµÄdataÄÚ´æ
         //ÄÚ´æ·ÖÅäÊ§°Ü°¡
        if (!frame_data)
        {
            LOGE("malloc audio frame failed ,we will lost one frame");
            if (rest_data)
                free (rest_data);
            rest_size = 0;
            continue; //Ö»ºÃÍË³öÖØÀ´Ò»´ÎÁË¡£
        }

		//ÉÏ´ÎÃ»ÓÃÍêµÄÊý¾Ý£¬ÒªÏÈ¸´ÖÆµ½ÄÚ´æÀïÍ·¡£
        if (rest_size)          // no rest data
        {
            LOGD("left %d byet last time\n", rest_size);
            memcpy (frame_data, rest_data, rest_size);
            free (rest_data);
            rest_data = NULL;
            memcpy (frame_data + rest_size, frame.data, frame.size);
        }

        frame_size = frame.size + rest_size;
        rest_size = 0;
        used = 0;
        declen = 0;

		//¼ÇÂ¼¶ÁÈ¡µ½µÄframeµ½in£¬ËÍÈ¥½âÂë
        pinfo->inptr = frame_data;
        pinfo->inlen = frame_size;
        pinfo->outlen = 0;

        //free pkt
        frame.data = NULL;
        frame.size = 0;

      DECODE_LOOP:
        if (decoder->status == ADEC_STATUS_EXIT)
        {
            LOGD("[%s:%d] receive decode loop exit cmd \n", __FUNCTION__, __LINE__);
            if (frame_data)
                free (frame_data);
            break;
        }
        /*decode frame */
        pinfo->consume = declen;
		//Ö´ÐÐ½âÂë
        used =ff_decode_audio_frame(decoder);// wrapper->decode_frame (wrapper, pinfo);
        if (used < 0)
        {
            decoder->decode_err_cnt++;
            /*
             * if decoder is ffmpeg,do not restore data if decode failed
             * if decoder is not ffmpeg, restore raw stream packet if decode failed
             * */
           // if (!strcmp (wrapper->name, "ffmpeg audio decoder"))
            {
                LOGE("[%s:%d] ffmpeg failed to decode this frame, just break", __FUNCTION__, __LINE__);
                decoder->decode_offset += pinfo->inlen;
            }
            continue;
        }//ÕâÊÇÉ¶ÒâË¼£¬usedÎª0£¬²¢ÇÒ±íÊ¾Ã»ÓÃÈ¥Êý¾Ýå
        else if (used == 0 && pinfo->outlen == 0) // used == 0 && out == 0 means need more data
        {
            //maybe need more data ,°ÑÕâ´ÎµÄÊý¾ÝÁôÆðÀ´£¬·Åµ½restdata´æ×Å
            rest_data = malloc (pinfo->inlen);
            if (rest_data == NULL)
            {
                LOGE("[%s:%d] rest_data malloc failed\n", __FUNCTION__, __LINE__);
                rest_size = 0;  //skip this frame
                continue;
            }
            memcpy (rest_data, pinfo->inptr, pinfo->inlen);
            rest_size = pinfo->inlen;
            LOGI("Maybe we need more data\n");
            continue;
        }
		//½âÂëok£¬ÓÃÈ¥ÁËpktÖÐused´óÐ¡µÄÊý¾Ý
        declen += used;
        pinfo->inlen -= used;
        decoder->decode_offset += used;
        decoder->pts_cache_size = pinfo->outlen;
        decoder->pts_buffer_size += pinfo->outlen;
        if (pinfo->outlen == 0)      //get no data, maybe first time for init
            LOGI("GET NO PCM DECODED OUT,used:%d \n",used);

        fill_size = 0;
      REFILL_BUFFER:
	  	//×´Ì¬»¹Ã»ÅªÄØ¡£
   //     if (decoder->status == ADEC_STATUS_EXIT)
   //         goto EXIT;
        /*write pcm */
        if (buf_space (out) < pinfo->outlen)
        {
          //  slogw("[%s:%d] decoder output buffer do not left enough space ,space=%d level:%d outsie:%d \n", __FUNCTION__, __LINE__, buf_space (out), buf_level (out), pinfo->outlen);
            usleep (1000);
            goto REFILL_BUFFER;
        }
        ret = buf_put (out, pinfo->outptr + fill_size, pinfo->outlen);
        fill_size += ret;
        pinfo->outlen -= ret;
        decoder->pts_cache_size = pinfo->outlen;
        if (pinfo->outlen > 0)
            goto REFILL_BUFFER;

        if (pinfo->inlen)
            goto DECODE_LOOP;
    }
    while (1);
  EXIT:
    //dt_info (TAG, "[file:%s][%s:%d]decoder loop thread exit ok\n", __FILE__, __FUNCTION__, __LINE__);
    LOGD("AFTER DECODER LOOP EXIT !");
    /* free adec_ctrl_t buf */
    if(pinfo->outptr)
        free(pinfo->outptr);
    pinfo->outlen = pinfo->outsize = 0;
    pthread_exit (NULL);
    return NULL;
}

int create_decoder_thread(audio_decoder_t *decoder)
{
	    /*create thread */
	pthread_t tid;
    int ret = pthread_create (&tid, NULL, audio_decode_loop, (void *) decoder);
    if (ret != 0)
    {
        LOGE("create audio decoder thread failed\n");
        ret = -1;
    }
	decoder->audio_decoder_pid = tid;
	return ret;
}
static init_decoder(audio_decoder_t *decoder)
{
	return ff_audio_init_decoder(decoder);
}

int createDecoder(ffmpeg_info_t *ff){
	int ret;
	if(ff==NULL){
		LOGE("get ff ptr fail");
		return -1;
	}
	audio_decoder_t *decoder;
	decoder=(audio_decoder_t *)malloc(sizeof(audio_decoder_t));
	if(decoder==NULL){
		LOGE("alloc decoder fail");
		return -1;
	}
	memset(decoder,0,sizeof(audio_decoder_t));
	//Òª×ªÎªvoid * Ã´?
	ff->decoder=decoder;
    decoder->parent=ff;
	//get from ff, not demuxer 20141215
	decoder->adec_in_queue=ff->pPktQueue;
	if(decoder->adec_in_queue==NULL){
		LOGE("get adec_in_queue ptr fail");
		return -1;
	}


	LOGI("before init_decoder");
	 ret = 	init_decoder(decoder);
	if(ret == 0){
		slogi("init_decoder ok !");
	}else{
		if(ret == -1 || ret == -2 )
		{
			LOGE("init_decoder fail !");
			ret = -1;
			goto ERR0;
		}

	}
	LOGI("AFTER init_decoder");
	ret= create_decoder_thread(decoder);
	if(ret == -1){
		LOGE("create decoder thread fail ");
		goto ERR1 ;
	}

	LOGI("AFTER create_decoder_thread");

	return 0;

	ERR1:
		buf_release(decoder->decoder_out_buf);
	ERR0:
		free(decoder);
	return -1;
}
