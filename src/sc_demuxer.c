#include"sc_error.h"
#include"sc_demuxer.h"
//#include <android/log.h>
//#include <string.h>
//#include"sc_thread_common.h"

static int ff_read_frame(AVFormatContext *pFmtCtx,AVPacket *pkt)
{
		//LOGD("before av_read_frame");
	    int ret=av_read_frame(pFmtCtx,pkt);
	//	LOGD("after av_read_frame,ret[%d]",ret);

		if(ret < 0)
    	{
			#if 1
			if(AVERROR(EAGAIN) != ret)
        	{
               /*if the return is EAGAIN,we need to try more times */
          	    LOGE("ERROR: av_read_frame return [%d]",ret);
            	if(AVERROR_EOF == ret || ret == -1)
				{
					LOGD("END OF FILE ERROR");
					return SCERROR_READ_EOF;
            	}
				else
				{
					LOGE(" read frame by demuxer fail !");
					return SCERROR_READ_FAILED;
    	    	}
    		}
			#endif
			return SCERROR_READ_AGAIN;
		}
		//¶ÁÈ¡ÕýÈ·
		return SCERROR_NONE;
}
//no static !!!
 void *audio_demux_loop(void *arg){
	audio_demuxer_t  *demuxer=(audio_demuxer_t  *)arg;
	demuxer->eof_flag=0;
	AVFormatContext *pFmtCtx=demuxer->priv;
	int ret;
	//¿ÉÒÔ¿¼ÂÇÓÃassert´úÌæ
	if(pFmtCtx == NULL){
		LOGE("demuxer->priv IS NULL");
		ret= -1;
		return;//Ö±½ÓÍË³ö°¡!
	}
	AVPacket pkt;
	int frame_valid=0;
	//LOGI("before while");
	while(!demuxer->eof_flag)
	{
	//	slogi("before ff_read_frame");
		//there is a bug inside
		ret=ff_read_frame(pFmtCtx,&pkt);
	//	slogi("after ff_read_frame");
	    if (ret == SCERROR_NONE)
            frame_valid = 1;//ÕâÊÇokµÄ
        else
        {
            if (ret == SCERROR_READ_EOF)
            {
				//´ËÊ±£¬Ó¦¸ÃÍË³ödemuxer
                demuxer->status = ADEMUX_STATUS_EXIT;
                demuxer->eof_flag = 1;
            }
			//ÄÑµÀÊÇµÈ´ýÍË³öÃ´å
            usleep (100);
            continue;
        }
		//Ó°Ïì¿´decoderµÄ±¨´í°¡
      //  slogd("ff_read_frame read size:%d pts:%lld \n",pkt.size,pkt.pts);
		if(pkt.stream_index == demuxer->ff_audio_stream_index){
			packet_queue_put(demuxer->ademx_queue,&pkt);
		}else
			av_free_packet(&pkt);
	}
	slogd("===================== of demuxer loop : audio_queue.size=%d\n",demuxer->ademx_queue->size);

}
int	create_demuxer_thread(audio_demuxer_t  *demuxer){
	slogi("%s begin",__FUNCTION__);
	pthread_t tid;
	int ret=pthread_create(&tid,NULL,audio_demux_loop,(void *)demuxer);
	if (ret != 0)
    {
        LOGE("create audio demuxer thread failed");
        ret = -1;
    }
	demuxer->audio_demuxer_pid= tid;
	slogi("%s end",__FUNCTION__);

	return ret;

}
static int format2bps (int fmt)
{
    int ret;
    switch (fmt)
    {
    case AV_SAMPLE_FMT_U8:
        ret = 8;
        break;
    case AV_SAMPLE_FMT_S16:
        ret = 16;
        break;
    case AV_SAMPLE_FMT_S32:
        ret = 32;
        break;
    default:
        ret = 16;
    }
    slogi("FORMAT2BPS FMT:%d bps:%d \n", fmt, ret);
    return ret;
}
static int getStreamInfo(ademx_streaminfo_t *pStrmInfo,AVStream *pAStrm,AVCodecContext *avctx)
{
	AVCodecContext *pCodecCtx=avctx;
	if(pCodecCtx==NULL)
	{
		LOGE("GET CODECCTX from AStream ERROR");
		return -1;
	}
	pStrmInfo->bit_rate=pCodecCtx->bit_rate;
	pStrmInfo->sample_rate=pCodecCtx->sample_rate;
	pStrmInfo->channels=pCodecCtx->channels;

	slogi("bitrate[%d] samplerate[%d] channels[%d]",pStrmInfo->bit_rate,pCodecCtx->sample_rate,pStrmInfo->channels);
	//¿ÉÒÔÔÚdecoderÀïÍ·»ñÈ¡µ½
	pStrmInfo->bps=format2bps(pCodecCtx->sample_fmt);
	slogi("pStrmInfo->bps[%d]",pStrmInfo->bps);
	//Òª²»Òª´Óchannel layout¼ÆËã»ñÈ¡å Õâ¸öÊ±ºòµÄµÄµ¥Î»ÊÇÃë°¡
	/*
	1.¶¨ÒåÔÚÍ·ÎÄ¼þstdint.hÖÐ
	2. Ê¹ÓÃprintfÊä³öÒ»¸öint64_tÕûÊýÊ±£¬¶ÔÓÚ32Î»ÏµÍ³£¬Ó¦Ê¹ÓÃ%lld£¬¶ÔÓÚ64Î»ÏµÍ³£¬Ó¦Ê¹ÓÃ%ld
	*/
	slogi("pAStrm->duration[%lld]",pAStrm->duration);
	pStrmInfo->duration=(int64_t)(pAStrm->duration*pAStrm->time_base.num)/pAStrm->time_base.den;
	pStrmInfo->time_base.num=pAStrm->time_base.num;
	pStrmInfo->time_base.den=pAStrm->time_base.den;
	//slogi("audio stream duration[%ld] timebase[%d %d] bps[%d] bit_rate[%d] sampele_rate[%d] channels[%d]",
	//	pStrmInfo->duration,pStrmInfo->time_base.num,pStrmInfo->time_base.den,pStrmInfo->bps,pStrmInfo->bit_rate,pStrmInfo->sample_rate,pStrmInfo->channels);


}
static int sc_decode_interrupt_cb(void *ctx)
{
    ffmpeg_info_t *is = ctx;
    return is->abort_request;
}
static int stream_component_open(ffmpeg_info_t *ff,int index,audio_demuxer_t *demuxer){
    AVFormatContext *ic = ff->fmt_ctx;
	AVCodecContext *avctx;
	AVCodec *codec;
	const char *forced_codec_name = NULL;
	int sample_rate, nb_channels;
    int64_t channel_layout;
    int ret;

	//avctx=ic->streams[index]->codec;
	slogi("audio stream index[%d]",index);
	AVStream *pAudioStream=ic->streams[index];
	avctx=pAudioStream->codec;
	codec=avcodec_find_decoder(avctx->codec_id);
	if(avctx->codec_type == AVMEDIA_TYPE_AUDIO){
		ff->last_audio_stream=index;
	//forced_codec_name=
	}else
	{
		sloge("#########NOT AUDIO!!!##############");
	}
	//pass to ff
	ff->dec_ctx=avctx;
	if(!codec){
		LOGE("no codec is found with codec_id[%d]",avctx->codec_id);
		return -1;
	}
	//ÎªÉ¶Òª×öÕâ¸ö¹¤×÷å£¿
	avctx->codec_id=codec->id;
	//pass to ff
	ff->dec=codec;
	//ÕâÐ©²ÎÊý
	sample_rate=avctx->sample_rate;
	nb_channels=avctx->channels;
	channel_layout=avctx->channel_layout;
	slogi("###sample_rate[%d] nb_channels[%d] channel_layout[%lld]",sample_rate,nb_channels,channel_layout);
	//ÓÃÒÔÉÏµÄ²ÎÊýÖ´ÐÐaudio open £¬ÎÒ·Åµ½audio decoderÀïÍ·È¥ÁË
	//Õâ¶ÎÄÚ´æÔÚÕâ·ÖÅä£¬²»Ò»¶¨ÓÐÈËÊÍ·Å°¡
	ademx_streaminfo_t *aStrmInfo=NULL;
	aStrmInfo=(ademx_streaminfo_t *)malloc(sizeof(ademx_streaminfo_t));
	if(aStrmInfo==NULL){
		LOGE("alloc streaminfo fail");
		return -1;
	}
#if 1
	/////////ÏÂÃæµÄÕâ¾ä»°»á±¨´íÃ´? ######ÏÂÃæÕâ¾ä»°ÓÐbug!!!!????
	memset(aStrmInfo,0,sizeof(ademx_streaminfo_t));
	//Ã»ÓÐÕâ¸öÒ²»á±¨´í
	getStreamInfo(aStrmInfo,pAudioStream,avctx);

	//pass to dmeuxer
	demuxer->astream_info=aStrmInfo;
#endif  ///////////end
	slogi("$$$$stream_component_open ok");
	return 0;

}
int init_demuxer(audio_demuxer_t **demuxer){
	*demuxer=(audio_demuxer_t *)malloc(sizeof(audio_demuxer_t));
	if(*demuxer==NULL)
	{
		LOGE("alloc demuxer fail");
		return -1;
	}
	memset(*demuxer,0,sizeof(audio_demuxer_t));
	return 0;
}
int uninit_demuxer(audio_demuxer_t *demuxer){
	if(demuxer==NULL){
		LOGE("NULL ,DO NOTHING ");
		return -1;
	}
	if(demuxer->astream_info)
		free(demuxer->astream_info);
	free(demuxer);
	demuxer=NULL;
	return 0;
}
int createDemuxer(char *filename,ffmpeg_info_t *ff){
	//ffmpeg_info.audio_stream_index		=-1;
    int ret;
    AVCodec *dec;
	AVFormatContext *pFmtCtx=NULL;
	//add this like ffplay 20141211
	//pFmtCtx = avformat_alloc_context();
	//pFmtCtx->interrupt_callback.callback=sc_decode_interrupt_cb;
	//pFmtCtx->interrupt_callback.opaque=pFmtCtx;

	//add pFmtCtx->iformat 20141211
	//AVInputFormat *iformat = NULL;
    if ((ret = avformat_open_input(&pFmtCtx, filename, NULL, NULL)) < 0) {
    	LOGE("Cannot open input file\n");
       /// return ret;
       goto FAIL;
    }

    if ((ret = avformat_find_stream_info(pFmtCtx, NULL)) < 0) {
          sloge("Cannot find stream information of [%s]",filename);
		  ret=-1;
         //  return ret;
         goto FAIL;
       }
	 /* select the audio stream */
	 int audio_stream_index;
	 //µÃµ½AVCodec,FFPLAY did not do this
    audio_stream_index = av_find_best_stream(pFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, /*&dec*/NULL, 0);
    if (audio_stream_index < 0) {
    	LOGE("Cannot find a audio stream in the input file\n");
        return ret;
    }
	//else // >=0 ¾ÍÊÇÕýÈ·ÕÒµ½£¬like ffplay
    //{
    	//pass to ff here
		ff->fmt_ctx=pFmtCtx;
		//pass to ff
		ff->audio_stream_index=audio_stream_index;
			//copy from ffplay
		 av_dump_format(pFmtCtx, 0, filename, 0);
			audio_demuxer_t *demuxer=NULL;
			//Ô­À´bugµÄÔ­ÒòÊÇÄÚ´æ·ÖÅäµÄÎÊÌâ°¡£¬Ö±½Ó´«µÝÖ¸Õë½øÈ¥ÊÇ²»ÐÐµÄ£¬Òª´«Ö¸ÕëµÄµØÖ·½øÈ¡,ÕâÑù²ÅÄÜÕæÕýÐÞ¸ÄÖ¸ÕëÖ¸ÏòµÄµØÖ·¡£
			if(init_demuxer(&demuxer)==-1){
				LOGE("alloc demuxer fail");
						goto FAIL;

			}
	if(demuxer==NULL){
		LOGE("AT LAST ,demuxer is NULL");
		goto FAIL;
	}

			//get codecctxºÍcodec etc here
		   ret= stream_component_open(ff,audio_stream_index,demuxer);
			if(ret == -1){
				LOGE("stream open fail ");
				goto FAIL;
			}


	//}
	slogi("$$$$after stream_component_open");
	//#if 1
   //demuxerµÄ¶ÓÁÐµÄ³õÊ¼»¯,Èç¹ûÔÚÕâÀï·ÖÅäÄÚ´æÒªË­À´ÊÍ·ÅÄØ?
   	PacketQueue *audio_queue=ff->pPktQueue;
	audio_queue=(PacketQueue *)malloc(sizeof(PacketQueue));
	if(audio_queue ==NULL){
		LOGE("alloc audioqueue fail");
		return -1;
	}
	packet_queue_init(audio_queue);
	slogi("$$$$after packet_queue_init");


/////////////Õâ¾ä»°»áÓÐbugÃ´å
	//	slogi("------1------");
	demuxer->filename=filename;

		//slogi("------2------");
	slogi("filename[%s]",demuxer->filename);
	//	slogi("------3------");

#if 1
	demuxer->parent=ff;
	demuxer->ademx_queue=audio_queue;
	//pass to ff
	ff->pPktQueue=audio_queue;
	//ÕâÁ©Ö¸ÏòÍ¬Ò»¸ö¶«¶«°¡
	demuxer->priv=pFmtCtx;
	demuxer->fmt_ctx=pFmtCtx;
#endif
//		slogi("------4------");

	demuxer->ff_audio_stream_index=audio_stream_index;//ff->audio_stream_index;
		//	slogi("------5------");

	slogi("ADUIO STREAM INDEX [%d]",demuxer->ff_audio_stream_index);

	//¿´À´ÊÇ¿ÉÒÔÍ¨¹ýdemuxerÀ´ÊÍ·ÅÄÚ´æµÄ
	//demuxer->astream_info=aStrmInfo;

	ff->demxuer=demuxer;

	slogi("========================create demuxer thread");
	create_demuxer_thread(demuxer);
	//#endif
   return ret;



   FAIL:
		if(ff->fmt_ctx){
			avformat_close_input(&ff->fmt_ctx);
		}
		ret=-1;
		return ret;
}


