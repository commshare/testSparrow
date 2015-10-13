#ifndef __TEST_CIR_BUF_H__
#define __TEST_CIR_BUF_H__

/* http://blog.csdn.net/roger_77/article/details/661859
�ֳ���ƽ̨�޹ص�������������

    ��C/C++�У����͵ĳ��ȸ���������أ���������ʵ��ȡ����CPU��
    ����TC++��DOS16�µ�Ӧ�ó���DOS16��16λ�Ĳ���ϵͳ��
    ����TC++��sizeof(int)==16��ͬ��win32��sizeof(int)==32��
    C99��׼����һ������<stdint.h>��ͷ�ļ���
    ��ͷ�ļ�������һϵ�и���������������typedef���֡�
    �����ٶ�C++����֧�ָ�ͷ�ļ��Ѿ���һ��ʱ���ˣ�������δ��ʽ��¼��C++��׼��
    ��ˣ���ʹ�ø�ͷ�ļ�֮ǰ����Ӧ�����Ķ���ı������ĵ����������Ƿ�֧�ָ��ļ�����ĳЩC++�ı�������,��GNC,Ϊ����ʹ��C99��׼����Щ������,����<inttypes.h>ͷ�ļ�������������ļ�<stdint.h>.
*/
#include<stdint.h>
typedef struct sc_cir_buf_s{
	//int rest;//ʣ��ռ�
	uint8_t *wptr;
	uint8_t *rptr;
	uint8_t *data;
	int size;//�ܴ�С
}cirbuf_t;

#endif
