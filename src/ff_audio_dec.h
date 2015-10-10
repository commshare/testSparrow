#ifndef __FF_AUDIO_DEC_H_
#define __FF_AUDIO_DEC_H_

#include"sc_audio_decoder.h"



int ff_decode_audio_frame (audio_decoder_t *decoder);
int ff_audio_init_decoder(audio_decoder_t *decoder);



#endif
