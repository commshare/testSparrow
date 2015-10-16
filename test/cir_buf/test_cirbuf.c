#include<stdio.h>
#include<stdlib.h>
#include <string.h>


#include"test_cirbuf.h"
#include"sc_log.h"


#define MIN(X,Y) ((X)>(Y)?(Y):(X))


/*
在put中，I II III 是可以put的位置。

data      r      w    data+size
-----------------------
     II             I

data      w      r     data+size
-----------------------
     V       III    IV
*/


/*希望用户传递的是一个地址*/
int cirbuf_init(cirbuf_t **buf,int max){
     /*改变地址指向的内容的值*/
     *buf=(cirbuf_t *)malloc(sizeof(cirbuf_t));
	 if(*buf==NULL)
	 {
	 	LOGD("malloc fail");
		return -1;
	 }
	 (*buf)->level=0;
	 memset(*buf,0,sizeof(cirbuf_t));
	 (*buf)->size=max;
     (*buf)->data=(uint8_t*)malloc(max*sizeof(uint8_t));
	 memset((*buf)->data,0,max*sizeof(uint8_t));
	 (*buf)->wptr=(*buf)->rptr=(*buf)->data;
	 return 0;

}
/*如果是基本数据，拷贝的是数据的值，如果是指针，拷贝的是实参的值（值是一个地址）。*/
void cirbuf_unint(cirbuf_t *buf){
    if(buf->data)
	    free(buf->data);
	buf->data=buf->rptr=buf->wptr=NULL;
    /*释放这个地址指向的内存*/
    free(buf);
	/*这改变的是形参吧，没其他作用*/
   //    buf=NULL;
}

int cirbuf_empty(cirbuf_t *buf)
{
	LOGD("buf->level [%d]",buf->level);
	if(buf->level==0)
		return 1;
//	return buf->level;
	return 0;
}
/*是不是循环缓冲不存在满的情况，可以直接覆盖的�*/
int cirbuf_full(cirbuf_t *buf){

	if(buf->level==buf->size)
	{
		LOGD("buff is full ");
		return 1;
	}
	return 0;

}
int cirbuf_put (cirbuf_t *buf,void *input,int put_size){
	LOGD("cirbuf_put [%d]",put_size);
	if(cirbuf_full(buf)){
		LOGD("full cirbuf ,put fail");
		return -1;
	}
	int rest=-1;
	int capability=-1;
	if(buf->wptr < buf->rptr){
		LOGD("W<R");
		rest=buf->rptr-buf->wptr;
		capability=MIN(rest,put_size);
		memcpy(buf->wptr,(uint8_t*)input,capability);
		buf->wptr+=capability;
		goto OK;
	}
	/*这应该是我的bug，W==R不一定就是在开头处，满的的时候无法put，所以W==R肯定是空*/
#if 0
       if(buf->data==buf->wptr && buf->data==buf->rptr){
#else
        if(buf->wptr==buf->rptr){
#endif
		LOGD("buf->data == W == R ");
		rest=buf->size;/*空*/
		capability=MIN(rest,put_size);
        memcpy(buf->wptr,(uint8_t*)input,put_size);
        buf->wptr=buf->data+capability;
		LOGD("after move capacity[%d] step: *(buf->wptr)[%c] *(buf->wptr-1)[%c]",capability,*(buf->wptr),*(buf->wptr-1));
        if(buf->wptr== (buf->data+buf->size)){/*表示已经写满*/
			LOGD("W at the end,from the begin 1");
			buf->wptr=buf->data;/*从0开始来了*/
		}
		goto OK;
	}

    if(buf->wptr > buf->rptr){
		LOGD("W>R");
        rest=(buf->size)-(buf->wptr-buf->rptr);
		//LOGD("*(buf->wptr-1)[%c]",*(buf->wptr-1));
		capability=MIN(rest,put_size);
		int I=(buf->data+buf->size) - buf->wptr;
		int II=0;
		LOGD("rest[%d] capablity[%d] I[%d]",rest,capability,I);
		int check=capability-I;
		/*因为插入要依赖于wptr，所以判断和插入是有先后顺序的，先插入I，移动wptr，
		此时，I不够用，使用移动后的wptr插入II，再次移动wptr，准备下次插入*/
		//LOGD("put capacity[%d] to I [%d] *(buf->wptr)[%c] ",capability,I,*(buf->wptr) );
		memcpy(buf->wptr,input,capability);
		buf->wptr+=capability;
		LOGD("buf->wptr[%c] buf->wptr+1 [%c]",*buf->wptr,*(buf->wptr+1));

		if(check==0 || check<0)/*I够用*/
		{
           LOGD("only I is enough");
		   if(check==0)
		   	 buf->wptr=buf->data;
		   goto OK;

		}else
		/*I不够用*/
		{
			LOGD("put II [%d]",II);
			II=capability-I;
			memcpy(buf->data,input-I,II);
			buf->wptr=buf->data+II;
		}
OK:
	    buf->level+=capability;
		return capability;//(I+II);
	}
}

int cirbuf_get(cirbuf_t *buf,void *output,int get_size){
  if(cirbuf_empty(buf)==1)
  {
  	  LOGD("cirbuf is empty,could not get");
	  return -1;
  }
  int availability=-1;
  int stored=-1;
  LOGD("do get[%d]",get_size);
  if(buf->wptr > buf->rptr){
  	LOGD("W>R");
	stored=buf->wptr-buf->rptr;
	goto OKOK;
  }else
  {
  	LOGD("W < R or W==R");
  	stored  =buf->size-(buf->rptr-buf->wptr);
	availability=MIN(stored,get_size);
    int IV=buf->data+buf->size - buf->rptr;
	int check=IV-availability;
	memcpy(output,buf->rptr,availability);
	buf->rptr+=availability;
	if(check<0){
		LOGD("output from V");
		/*已经写入了IV大小*/
		memcpy(output+IV,buf->data,(availability-IV));
	}
    #if 0
	else{
		if(check==0){/*存在这么一种刚好够用的情况*/
	   		LOGD("rptr move to beginning");
			buf->rptr=buf->data;
	   }
	}
	#endif
	goto OK;

  }
OKOK:
	availability=MIN(stored,get_size);
	memcpy(output,buf->rptr,availability);
	buf->rptr+=availability;
 OK:
	buf->level-=availability;
	if(buf->rptr == (buf->data+buf->size)){
		LOGD("read to end ,from the begin  ");
		buf->rptr=buf->data;
	}
	return availability;
}

#define TEST_PUT 0
#define TEST_OUT 1

int main(){
	int i;
	cirbuf_t *buf;
    cirbuf_init(&buf,10);
	uint8_t *array="1234567890";
    uint8_t *array2="abcdefghik";
	/*写入1到0，共10个*/
	cirbuf_put(buf,array,10);
	/*uint8_t 居然只占了一个字节，和char一样*/
	LOGD("sizeof(int)[%d] sizeof(uint8_t)[%d] sizeof(char)[%d]",sizeof(int),sizeof(uint8_t),sizeof(char));
	LOGD("----1---AFTER PUT");

	for(i=0;i<10;i++)
		printf("buf->data[%c] \n",*(buf->data+i));
#if TEST_PUT

	/*此时，应该用array2全部覆盖了当前缓冲，即输出a到k*/
	cirbuf_put(buf,array2,10);
	LOGD("----2---AFTER PUT");
	for(i=0;i<10;i++)
		printf("buf->data[%c] \n",*(buf->data+i));

	/*写入4个数字，前面四个是1234，后面是efghik*/
	cirbuf_put(buf,array,4);
	LOGD("----3---AFTER PUT");
	for(i=0;i<10;i++)
		printf("buf->data[%c] \n",*(buf->data+i));


	/*再写入4个字母，应该是1234abcdik*/
	cirbuf_put(buf,array2,4);
	LOGD("----4---AFTER PUT");
	for(i=0;i<10;i++)
		printf("buf->data[%c] \n",*(buf->data+i));

    /*空间不足，写入两个数字12，1234abcd12*/
	cirbuf_put(buf,array,7);
	LOGD("----5---AFTER PUT");
	for(i=0;i<10;i++)
		printf("buf->data[%c] \n",*(buf->data+i));

    /*abcdefgd12*/
	cirbuf_put(buf,array2,7);
	LOGD("----6---AFTER PUT");
	for(i=0;i<10;i++)
		printf("buf->data[%c] \n",*(buf->data+i));

	/*abcdefg123*/
	cirbuf_put(buf,"#",1);
	LOGD("----7---AFTER PUT");
	printf("buf->data[%s] \n",(buf->data));
#endif

#if TEST_OUT
    uint8_t * out=malloc(10*sizeof(uint8_t));
    if(out==NULL){
		LOGD("malloc fail");
		return -1;
	}
    memset(out,0,10*sizeof(uint8_t));
    cirbuf_get(buf, out, 4);
	LOGD("out[%s]",out);
	cirbuf_get(buf, out, 4);
	LOGD("out[%s]",out);
	cirbuf_get(buf, out, 2);
	LOGD("out[%s]",out);
	cirbuf_get(buf, out, 4);
	LOGD("out[%s]",out);
	cirbuf_put(buf,array2,4);
	LOGD("buf->data[%s]",buf->data);
	cirbuf_put(buf,array2,4);
	LOGD("buf->data[%s]",buf->data);
	cirbuf_put(buf,array2,4);
	LOGD("buf->data[%s]",buf->data);
	cirbuf_put(buf,array,4);
	LOGD("buf->data[%s]",buf->data);
	cirbuf_get(buf, out, 4);
	LOGD("out[%s]",out);
	LOGD("get from buf->data[%s]",buf->data);
	cirbuf_get(buf, out, 7);
	LOGD("then out[%s]",out);
	LOGD("get from buf->data[%s]",buf->data);
	cirbuf_get(buf, out, 7);
	LOGD("then out[%s]",out);
	free(out);
#endif

	cirbuf_unint(buf);
    return 0;
}
