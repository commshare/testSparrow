#ifndef _SC_AUDIO_OUT_H_
#define _SC_AUDIO_OUT_H_

#include"sc_thread_common.h"
#include"sc_buffer.h"
#include"sc_main.h"

#define PCM_WRITE_SIZE 10 // ms
typedef enum
{
    AO_STATUS_IDLE, //����
    AO_STATUS_PAUSE,  //��ͣ
    AO_STATUS_RUNNING,  //����ing
    AO_STATUS_EXIT,  //�˳�
} aout_status_t;

typedef struct aout_param{
int sample_rate;
int channels;
//������ʽ,�������Բ�Ҫ�ġ������PCM��ʽ�ǹ̶���

int bps;
int data_width;//

}aout_param_t;

typedef struct audio_out{
aout_param_t param;
void *priv; //aout_sl_t
void *parent;

sc_buf_t *adec_out_buf; //use pointer
aout_status_t status;

pthread_t aout_thread_id;
}audio_out_t;
int create_aout(ffmpeg_info_t *ff);


#endif
