#include"ff_audio_dec.h"
#include"ff_audio_dec_internal.h" //internal useage

#include"sc_log.h"

#include"sc_edian.h"
#include"sc_def.h"

#include"sc_demuxer.h"

static AVFrame *frame; //只需要分配一次内存就ok

int	init_decoder_input(){
    //alloc one frame for decode  在这里分配空间，用于解码。原来这个frame代表的是解码用的。
    frame = av_frame_alloc ();
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
static int get_adec_ctx(audio_decoder_t *decoder){

	/**
 * Allocate an AVCodecContext and set its fields to default values. The
 * resulting struct should be freed with avcodec_free_context().
 *
 如果给了codec就按照这个codec来初始化ctx，赋给它默认的值，
 此后不可以再调avcodec_open打开一个其他的codec了。
 * @param codec if non-NULL, allocate private data and initialize defaults
 *              for the given codec. It is illegal to then call avcodec_open2()
 *              with a different codec.
 如果是空的，那么codec相关的域都不会默认的呗初始化的。
 *              If NULL, then the codec-specific defaults won't be initialized,
 *              which may result in suboptimal default settings (this is
 *              important mainly for encoders, e.g. libx264).
 *
 * @return An AVCodecContext filled with default values or NULL on failure.
 * @see avcodec_get_context_defaults
 */
 AVCodecContext *pDecCtx;
 	#if 0
   //AVCodecContext *avcodec_alloc_context3(const AVCodec *codec);
	AVCodecContext *pDecCtx=avcodec_alloc_context3(NULL);
	#else
	//这里要是能改成一个类用就好了
	//((ffmpeg_info *)(decoder->parent))->fmt_ctx;
	//通过demuxer来找到fmtctx，而不是直接走ffmepginfo
	ffmpeg_info_t * ff=(ffmpeg_info_t *)decoder->parent;
	audio_demuxer_t *demuxer=(audio_demuxer_t *)ff->demxuer;
	if(demuxer ==NULL){
		LOGE("ff decoder get ffinfo demuxer fail !");
		return -1;
	}
	AVFormatContext *pFmtCtx= demuxer->fmt_ctx;
   //    ffmpeg_info.dec_ctx = ffmpeg_info.fmt_ctx->streams[ffmpeg_info.audio_stream_index]->codec;
   	if(pFmtCtx==NULL){
		LOGE("get fmtctx fail ! ");
		return -1;
	}
	//干脆把寻找codec的事儿交给demuxer?
    //  pDecCtx= pFmtCtx->streams[demuxer->ff_audio_stream_index]->codec;
    pDecCtx=ff->dec_ctx;//demuxer 负责
	#endif
	if(pDecCtx==NULL){
		LOGE("get AVCodecContext fail");
		return -1;
	}
	//AVFormatContext *pFmtCtx=((ffmpeg_info *)(decoder->parent))->fmt_ctx;

	//pDecCtx->codec_type=AVMEDIA_TYPE_AUDIO;
	//pDecCtx->codec_id=pFmtCtx->


	/*
    if ((ret = avcodec_open2(ffmpeg_info.dec_ctx, dec, NULL)) < 0) {
    	LOGE("Cannot open audio decoder\n");
        return ret;
    }
	*/
	    /* init the audio decoder */
	LOGD("codec_id [%d] should be 0 ?",pDecCtx->codec_id);
	int ret;
	AVCodec *dec=ff->dec;
    if ((ret = avcodec_open2(pDecCtx, dec, NULL)) < 0) {
    	LOGE("Cannot open audio decoder\n");
        return ret;
    }
	//pDecCtx->codec_id=dec->id;
	slogd("after avcodec_open : dec codec_id [%d] and avctx codec_id[%d]",dec->id,pDecCtx->codec_id);
	//pass to decoder
	decoder->priv=pDecCtx;

	return 0;
}
static int init_decoder_out_buffer(audio_decoder_t *decoder)
{
	int size = DTAUDIO_PCM_BUF_SIZE;
	int ret = buf_init (&decoder->decoder_out_buf, size);
	if(ret < 0)
	{
		ret=-1;
		//goto ERR1;
	}
//ERR1:
	//会要求对audio_decoder_t 进行资源释放

    return ret;
}
static int release_decoder_out_buffer(audio_decoder_t *decoder){
	return buf_release(&decoder->decoder_out_buf);
}

static int audio_open2(audio_decoder_t *decoder, int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate)
{
	slogi("SRC : wanted_channel_layout[%lld],wanted_nb_channels[%d],wanted_sample_rate[%d]",wanted_channel_layout,wanted_nb_channels,wanted_sample_rate);
	 if (!wanted_channel_layout || wanted_nb_channels != av_get_channel_layout_nb_channels(wanted_channel_layout)) {
        wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
        wanted_channel_layout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
    }
	 //还是对src做了修改
	decoder->param.src_channels=av_get_channel_layout_nb_channels(wanted_channel_layout);
	decoder->param.src_samplerate=wanted_sample_rate;
	if(decoder->param.src_channels <= 0 || decoder->param.src_samplerate<=0 )
	{
		LOGE( "Invalid sample rate or channel count!\n");
        return -1;
	}
    LOGD("dst  param: src_channels[%d],src_samplerate[%d]",decoder->param.src_channels,decoder->param.src_samplerate);
	return 0;
}


//static
//for export
int ff_audio_init_decoder(audio_decoder_t *decoder){

	AVCodecContext *pDecCtx=NULL;
	if(decoder->priv==NULL)
	{
		int ret=get_adec_ctx(decoder);
		if(ret == -1 )
		{
			LOGE("ff decoder get adec ctx fail!");
			return -1;
		}
	}
     pDecCtx=decoder->priv;

    decoder->pts_current = decoder->pts_first = -1;
	//为啥都设置为1�
	if(decoder->param.den==0 || decoder->param.num==0)
		decoder->param.den=decoder->param.num=1;
	else{
		  LOGD("WHAT ?");
		}

	//直接在这里得到
	decoder->param.data_width=format2bps(pDecCtx->sample_fmt);
	LOGD("DECODER data_width[%d]",decoder->param.data_width);
	//这是直接从avctx里头取出来啊，之前demexuer里头取出来一次
    int    sample_rate    = pDecCtx->sample_rate;
    int    nb_channels    = pDecCtx->channels;
    int64_t   channel_layout = pDecCtx->channel_layout;
	slogi("source wanted : sample_rate[%d] nb_channels[%d] channels_layout[%lld]",sample_rate,nb_channels,channel_layout);
	/* prepare audio output */
	int ret=audio_open2(decoder,channel_layout, nb_channels, sample_rate);
	if(ret == -1 ){
		LOGE("ADUIO OPEN 2 FAIL !");
		goto ERR0;
	}
	init_decoder_input();
	ret = init_decoder_out_buffer(decoder);
	if(ret == -1){
		LOGE("create decoder out buffer fail ,goto err1 ");
		goto ERR1;//释放掉decoder

	}
	#if 0
	ret= create_decoder_thread(decoder);
	if(ret == -1){
		LOGE("create decoder thread fail ,goto ERR2");
		goto ERR2: //释放掉decoder_out_buf
	}
	#endif
	return 0;
	ERR0:
		return -1;
	ERR1:
		//release_decoder();
		//让上一层来处理�
		return -1;
	ERR2:
		release_decoder_out_buffer(decoder);
		return -2;

}

#if 0
//, struct AudioParams *audio_hw_params
static int audio_open1(audio_decoder_t *decoder, int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate){
{
	SC_AudioSpec wanted_spec,spec;
    if (!wanted_channel_layout || wanted_nb_channels != av_get_channel_layout_nb_channels(wanted_channel_layout)) {
        wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
        wanted_channel_layout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
    }
	//返回channel layout里头有多少个channel
	wanted_spec.channels = av_get_channel_layout_nb_channels(wanted_channel_layout);
    wanted_spec.freq = wanted_sample_rate;
	//这肯定是错误的
    if (wanted_spec.freq <= 0 || wanted_spec.channels <= 0) {
        LOGE( "Invalid sample rate or channel count!\n");
        return -1;
    }
	#if 0
	if(SC_BYTEORDER == SC_LIL_ENDIAN)
		LOGD("we are littile edian");
	else
		LOGD("we are big edian");

	//格式是写死的
	wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.silence = 0;
    wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE; //1024个采样点?

 //   wanted_spec.callback = sdl_audio_callback;
  //  wanted_spec.userdata = opaque;
    #endif
}
#endif

static int64_t pts_exchange (audio_decoder_t * decoder, int64_t pts)
{
    return pts;
}

//解码完毕之后要转换
static void audio_convert (audio_decoder_t *decoder, AVFrame * dst, AVFrame * src)
{
    int nb_sample;
    int dst_buf_size;
    int out_channels;
    //for audio post processor
    //struct SwsContext *m_sws_ctx = NULL;
    struct SwrContext *m_swr_ctx = NULL;
    //ResampleContext *m_resample_ctx=NULL;
    AVCodecContext *avctxp=decoder->priv;
    enum AVSampleFormat src_fmt = avctxp->sample_fmt;
	//强制使用固定的格式
    enum AVSampleFormat dst_fmt = AV_SAMPLE_FMT_S16;

    dst->linesize[0] = src->linesize[0];
	//这是要干啥??????????
    *dst = *src;

    dst->data[0] = NULL;

	//在这里做了修改
	if(decoder->param.src_channels>2)
	{
		out_channels =2;//decoder->aparam.dst_channels;
		LOGD("SRC channels is [%d] ,we make it to 2",decoder->param.src_channels);
	}

	//解码之后就一帧里头有多少个采样点了。
    nb_sample = src->nb_samples;//frame->nb_samples;

    dst_buf_size = nb_sample * av_get_bytes_per_sample (dst_fmt) * out_channels;
    dst->data[0] = (uint8_t *) av_malloc (dst_buf_size);

    avcodec_fill_audio_frame (dst, out_channels, dst_fmt, dst->data[0], dst_buf_size, 0);
    LOGD("before convert :SRCFMT:%d dst_fmt:%d \n", src_fmt, dst_fmt);
    /* resample toAV_SAMPLE_FMT_S16 */
    if (src_fmt != dst_fmt || /*out_channels != decoder->aparam.channels*/ decoder->param.src_channels>2)
    {
        if (!m_swr_ctx)
        {
            uint64_t in_channel_layout = av_get_default_channel_layout (avctxp->channels);
            uint64_t out_channel_layout = av_get_default_channel_layout (out_channels);
            m_swr_ctx = swr_alloc_set_opts (NULL, out_channel_layout, dst_fmt, avctxp->sample_rate, in_channel_layout, src_fmt, avctxp->sample_rate, 0, NULL);
            swr_init (m_swr_ctx);
        }
        uint8_t **out = (uint8_t **) & dst->data;
        const uint8_t **in = (const uint8_t **) src->extended_data;
        if (m_swr_ctx)
        {
            int ret, out_count;
            out_count = nb_sample;
            ret = swr_convert (m_swr_ctx, out, out_count, in, nb_sample);
            if (ret < 0)
            {
                //set audio mute
                memset (dst->data[0], 0, dst_buf_size);
                LOGE ("####audio convert failed, set mute data");
            }
        }
    }
    else                        // no need to convert ,just copy
    {
        memcpy (dst->data[0], src->data[0], src->linesize[0]);
    }
    //free context
    if (m_swr_ctx != NULL)
        swr_free (&m_swr_ctx);
    //if(m_resample_ctx!=NULL)
    //    audio_resample_close(m_resample_ctx);
}

//1 get one frame 0 failed -1 err
int ff_decode_audio_frame (audio_decoder_t *decoder)
{
    int got_samples = 0;
    int ret = 0;
    int data_size = 0;
    AVFrame frame_tmp;
	//static
	//AVFrame *frame;
    if(!frame)
    {
		LOGD("alloc frame for decoder now ");
	    if (!(/*frame = avcodec_alloc_frame()*/	frame = av_frame_alloc ()))
		    {
				LOGE("alloc frame for decoder fail");
				return AVERROR(ENOMEM);
	    	}
    }
	else
	{
		/**
	 	* Unreference all the buffers referenced by frame and reset the frame fields.
		 */
		//https://github.com/kierank/x262/pull/6/files?diff=split
	//	#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55,28,1)
		    av_frame_unref( frame );
	//	#else
     //		avcodec_get_frame_defaults(frame);
	//	#endif

	}

    //dtaudio_decoder_t *decoder = (dtaudio_decoder_t *)wrapper->parent;
     adec_ctrl_t *pinfo=&(decoder->info);
    AVPacket pkt;
	//dttv居然是这么用的，这内存是导出了?
    pkt.data = pinfo->inptr;
    pkt.size = pinfo->inlen;
    pkt.side_data_elems = 0;

	//pass to decoder when init ctx
	AVCodecContext *avctxp=(AVCodecContext *)decoder->priv;
	if(avctxp == NULL){
		LOGE("get avctxp from decoder fail");
		return -1;
	}
    memset (&frame_tmp, 0, sizeof (frame_tmp));
    slogd("1start decode size:%d %02x %02x ", pkt.size, pkt.data[0], pkt.data[1]);
	//看来这个函数会把pkt的size大小的data数据解码，而后返回frame，
    ret = avcodec_decode_audio4 (avctxp, frame, &got_samples, &pkt);
    slogd("2start decode size:%d %02x %02x %02x %02x \n", pkt.size, pkt.data[0], pkt.data[1], pkt.data[2], pkt.data[3]);
    if (ret < 0)
    {
        LOGE("decode failed ret:%d \n", ret);
        goto EXIT;
    }

    if (!got_samples)           //decode return 0
    {
        LOGE("get no samples out \n");
        pinfo->outlen = 0;
        goto EXIT;
    }
	/**
 * Get the required buffer size for the given audio parameters.
 *
 * @param[out] linesize calculated linesize, may be NULL 这个是要输出的
 * @param nb_channels   the number of channels
 * @param nb_samples    the number of samples in a single channel 单个channel的采样点数目
 * @param sample_fmt    the sample format 采样格式
 * @param align         buffer size alignment (0 = default, 1 = no alignment)  缓冲是否对齐
 * @return              required buffer size, or negative error code on failure
 */
    data_size = av_samples_get_buffer_size (frame->linesize, avctxp->channels, frame->nb_samples, avctxp->sample_fmt, 1);
    if (data_size > 0)
    {
		LOGD("BEFORE AUDIO CONVERT");
		//重采样啊
        audio_convert (decoder, &frame_tmp, frame);
		LOGD("after AUDIO CONVERT");

        //out frame too large, realloc out buf
        if(pinfo->outsize < frame_tmp.linesize[0])
        {
			//重新分配内存空间
            pinfo->outptr = realloc(pinfo->outptr,frame_tmp.linesize[0] * 2);
            pinfo->outsize = frame_tmp.linesize[0] * 2;
        }

		//把转换后的frame_tmp导入到out中来。
        memcpy (pinfo->outptr, frame_tmp.data[0], frame_tmp.linesize[0]);
		//这个outlen如果为0，有可能是convert的问题吧!!!!!!
        pinfo->outlen = frame_tmp.linesize[0];
    }
    else //av_samples_get_buffer_size 出错了!
    {
        LOGE( "[av_samples_get_buffer_size  fail ! ]data_size invalid: size:%d outlen:%d \n", data_size, pinfo->outlen);
        pinfo->outlen = 0;
    }

  EXIT:
    if (frame_tmp.data[0])
        free (frame_tmp.data[0]);
    frame_tmp.data[0] = NULL;
	//返回解码用掉了pkt中多少数据了
    return ret;
}

#if 0
AVPacket packet;
AVFrame *frame;
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
