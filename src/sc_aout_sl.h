#ifdef  _SC_OUT_SL_H_
#define  _SC_OUT_SL_H_



/**
 * Convert OpenSL ES result to string.
 *
 * @param result result code.
 * @return result string.
 */
//static const char* ResultToString(SLresult result);


//static SLuint32 convertSampleRate(SLuint32 sr);
//int create_sl_engine(audio_out_t *aout);
//int create_sl_player(audio_out_t *aout);
//static
//void stop_sl_player(audio_out_t *aout);
int aout_sl_write (audio_out_t *aout, uint8_t * buf, int size);

int aout_sl_stop(audio_out_t *aout);
int aout_sl_init(audio_out_t *aout);

#endif
