#ifndef __TEST_CIR_BUF_H__
#define __TEST_CIR_BUF_H__

/* http://blog.csdn.net/roger_77/article/details/661859
字长与平台无关的整型数据类型

    在C/C++中，整型的长度跟编译器相关，编译器的实现取决于CPU。
    比如TC++是DOS16下的应用程序，DOS16是16位的操作系统，
    所以TC++中sizeof(int)==16；同理win32中sizeof(int)==32。
    C99标准定义一个叫着<stdint.h>的头文件，
    该头文件定义了一系列各种类别的整数类型typedef名字。
    尽管速多C++工具支持该头文件已经有一段时间了，但它尚未正式收录于C++标准，
    因此，在使用该头文件之前，你应该先阅读你的编译器文档，看看它是否支持该文件。在某些C++的编译器中,如GNC,为了能使用C99标准的这些新特性,就在<inttypes.h>头文件中引入了这个文件<stdint.h>.
*/
#include<stdint.h>
typedef struct sc_cir_buf_s{
	//int rest;//剩余空间
	uint8_t *wptr;
	uint8_t *rptr;
	uint8_t *data;
	int size;//总大小
}cirbuf_t;

#endif
