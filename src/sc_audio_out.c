#include"sc_audio_out.h"

#include"sc_aout_sl.h"

#include"sc_audio_decoder.h"

static int aout_write_frame(audio_out_t *aout,uint8_t *buffer,int size){
return	aout_sl_write(aout,buffer,size);
}
static int aout_read_frame(audio_out_t *aout,uint8_t *buffer,int unit_size){
	return buf_get(aout->adec_out_buf,buffer,unit_size);
}
static void *aout_thread_loop (void *arg){
	audio_out_t *aout=(audio_out_t *)arg;
	if(aout==NULL){
		LOGE("aout==NULL fail");
		return ;
	}
	#if 0
	aout_param_t *param=NULL;
	param=&aout->param;
	if(param== NULL){
		LOGE("get aout param fail ");
		return ;
	}
	#endif
	//slogi("%s : param->data_width[%d] param->channels[%d]",__FUNCTION__,param->data_width,param->channels);
	slogi("%s : param->data_width[%d] param->channels[%d]",__FUNCTION__,aout->param.data_width,aout->param.channels);
	//int bytes_per_sample= param->data_width * param->channels / 8/*◊÷Ω⁄*/ ;
	int bytes_per_sample= aout->param.data_width * aout->param.channels;
	//Œ™…∂“™≥˝“‘1000ƒÿ?
	slogi("caculate unit_size: param->sample_rate[%d] bytes_per_sample[%d]",aout->param.sample_rate,bytes_per_sample);
	const int unit_size = PCM_WRITE_SIZE *aout->param.sample_rate * bytes_per_sample /1000/*±‰≥…√Î*/ ;
	LOGI("AOUT: write :%d ms :%d bytes one time ",PCM_WRITE_SIZE, unit_size);
	if(unit_size == 0){
		LOGE("ERROR ! unit_size is 0 ");
		return;
	}
	uint8_t *buffer=malloc(unit_size);
	if(!buffer)
	{
		LOGE("malloc fail");
		goto EXIT;
	}
	int read_len=0,write_len=0;
	slogi("before audio out loop...");
	for(;;){
		if(read_len <= 0){
			read_len=aout_read_frame(aout,buffer,unit_size);
			if(read_len < 0 ){
				sloge("aout read frame fail");
				usleep (1000);
				continue;
			}
		}
		//too many print 20141216
		//slogi("before aout write frame");
		write_len=aout_write_frame(aout,buffer,read_len);
		read_len -= write_len;
		if(read_len > 0)
			memmove(buffer,buffer+write_len,read_len);
		write_len=0;
	}
EXIT:
	LOGD("AOUT THREAD EXIT ! ");
	if(buffer)
		free(buffer);
	buffer=NULL;
	pthread_exit(NULL);
	return NULL;

}
static int init_audio_parm(audio_out_t *aout){
	 ffmpeg_info_t *ff=aout->parent;
	if(ff == NULL){
		LOGE("aout get ffinfo error !");
		return -1;
	}
	audio_decoder_t *decoder=(audio_decoder_t *)ff->decoder;

	int channels=decoder->param.src_channels;
	//audio_para.data_width = host_para->audio_sample_fmt;
	int data_width=decoder->param.data_width;
	int sample_rate=decoder->param.src_samplerate;
	int bps=decoder->param.bps; //’‚∏ˆ «0 √¥Â

	aout->param.channels=channels;
	aout->param.data_width=data_width;
	aout->param.bps=bps;
	aout->param.sample_rate=sample_rate;
	slogi("NOW aout audioparam: channels[%d] data_width[%d] bps[%d] sample_rate[%d]",
		aout->param.channels,aout->param.data_width,aout->param.bps,aout->param.sample_rate);
	return 0;

}
static int aout_start(audio_out_t *aout){
	pthread_t tid;
	int ret=0;//means ok !
	#if 1
	slogi("before create aout_thread_loop");
	slogi("AGAIN aout audioparam: channels[%d] data_width[%d] bps[%d] sample_rate[%d]",aout->param.channels,aout->param.data_width,aout->param.bps,aout->param.sample_rate);
	ret=pthread_create(&tid,NULL,aout_thread_loop,(void *)aout);
	if(ret != 0){
		LOGE("create aout thread fail !");
		ret = -1 ;//means thread create fail !
		//goto ERR_AOUT_THREAD_FAIL;
		return ret;
	}
	aout->aout_thread_id=tid;
	#endif
	slogi("create aout thread ok !");
	return 0;

}
static int aout_init(audio_out_t *aout){
	int ret=0;//means ok !
	init_audio_parm(aout);
	slogi("THEN aout audioparam: channels[%d] data_width[%d] bps[%d] sample_rate[%d]",aout->param.channels,aout->param.data_width,aout->param.bps,aout->param.sample_rate);
    ret= aout_sl_init(aout);
	if(ret == -1 ){
		LOGE("aout_sl_init fail");
		goto ERR_SL_FAIL;
	}

	return 0;

#if 0
ERR_AOUT_THREAD_FAIL: //create aout thread fail
		//”¶∏√ Õ∑≈¥¥Ω®≥…π¶µƒsl◊ ‘¥
		//’‚∏ˆ”¶∏√æÕ «÷±Ω”ÕÀ≥ˆsl¡À
		aout_sl_stop(aout);

#endif
ERR_SL_FAIL://create sl fail
		//“ÚŒ™sl…Ëº∆µƒ‘µπ £¨»Áπ˚slπ“¡À£¨ƒ«√¥sl◊‘º∫ª·◊ˆ«Â¿Ìπ§◊˜°£
	return -1;


}
int aout_stop(audio_out_t *aout){
	aout->status=AO_STATUS_EXIT; //ÕÀ≥ˆ
	//Ω· ¯œﬂ≥Ã
	pthread_join(aout->aout_thread_id,NULL);
	//call sl stop ,sl_stop ƒ⁄≤ø”–º”À¯
	aout_sl_stop(aout);

	return 0;
}
int create_aout(ffmpeg_info_t *ff){
	int ret;
	audio_out_t *aout=(audio_out_t*)malloc(sizeof(audio_out_t));
	if(aout==NULL){
		ret=-1;
		LOGE("alloc aout fail");
		return ret;
	}
	memset(aout,0,sizeof(audio_out_t));

	//pass to ff
	ff->aout=(void *)aout;
	aout->parent=ff;
	audio_decoder_t *decoder=(audio_decoder_t *)(ff->decoder);
	//pass from decoder to aout
	if(ff->decoder != NULL){
		aout->adec_out_buf=&(decoder->decoder_out_buf);
	}else
	{
		LOGE("decoder is NULL !!! fail to get aout buf!");
		//return -1;
		goto ERR0;
	}
	aout->status=AO_STATUS_IDLE;
	/////////////////
	slogi("before aout_init");
	 ret=aout_init(aout);
	if(ret == -1 )
	{
		LOGE("aout_init FAIL ");
		goto ERR_AOUT_INIT_FAIL; //SL FAIL
	}else
	{
		slogi("aout_init ok");
		slogd("channels[%d] sample_rate[%d]",aout->param.channels,aout->param.sample_rate);
	}
//	aout->status=AO_STATUS_RUNNING;
	/////////////////

	//œﬂ≥Ã‘⁄’‚¿Ô¥¥Ω®
	ret=aout_start(aout);
	if(ret== -1){
		LOGE("aout_start fail ");
		goto ERR_AOUT_START_FAIL; //THREAD FAIL
	}else
		slogi("aout_start ok");

	aout->status=AO_STATUS_RUNNING;
	return 0;

	ERR_AOUT_INIT_FAIL:
		//SL  will take care of themselves,so do nothing
	ERR_AOUT_START_FAIL:
		//stop SL
		LOGD("aout start fail £¨do aout_sl_stop ");
		aout_sl_stop(aout);
	ERR0:
		//release aout
		aout->adec_out_buf=NULL;
		free(aout);
		return -1;
}

