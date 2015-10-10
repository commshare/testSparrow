#ifndef __SC_AUDIO_DECODER_H_
#define __SC_AUDIO_DECODER_H_

#include"sc_main.h"
#include"sc_buffer.h"
#include"sc_av.h" //for dtratio

typedef struct
{
    int index;
    int id;
    int bit_rate; //文件的比特率可以从fmt里头拿到
    int sample_rate;
    int channels;
    int bps;
    int64_t duration; //这个应该也可以拿到
    dtratio time_base;
    int extradata_size;
    uint8_t *extradata;
  //  dtaudio_format_t format;  //aac ac3
    void *codec_priv;
} astream_info_t;

#define AUDIO_EXTRADATA_SIZE 4096
typedef struct AudioParams {
    int freq;
    int channels;
    int64_t channel_layout;
    enum AVSampleFormat fmt;
    int frame_size;
    int bytes_per_sec;
} AudioParams;
typedef struct audio_param{

    int src_channels,dst_channels;
    int src_samplerate,dst_samplerate;
    int data_width;
    int bps;
    int num, den;
    int extradata_size;
    unsigned char extradata[AUDIO_EXTRADATA_SIZE];
}audio_param_t;
typedef enum
{
    ADEC_STATUS_IDLE,
    ADEC_STATUS_RUNNING,
    ADEC_STATUS_PAUSED,
    ADEC_STATUS_EXIT
} adec_status_t;
typedef struct{
    uint8_t *inptr;
    int inlen;
    int consume;
    uint8_t *outptr;
    int outsize; // buffer size
    int outlen;  // buffer level

    int info_change;
    int channels;
    int samplerate;
    int bps;
}adec_ctrl_t;
typedef struct audio_decoder
{
   // dtaudio_para_t aparam;
   // ad_wrapper_t *wrapper;
    pthread_t audio_decoder_pid;
    adec_status_t status;
    int decode_err_cnt;
    int decode_offset;

	////pts
    int64_t pts_current;
    int64_t pts_first;
    int64_t pts_last_valid;
    int pts_buffer_size;
    int pts_cache_size;

     adec_ctrl_t info;
	 PacketQueue *adec_in_queue;
    sc_buf_t decoder_out_buf;
    void *parent;
    void *priv;         //point to avcodeccontext

	//存储格式、codec等信息。
	audio_param_t param;
	//struct AudioParams audio_tgt;
    struct SwrContext *swr_ctx;
}audio_decoder_t;
int createDecoder(ffmpeg_info_t *ff);
#endif
