#include"sc_buffer.h"
#include"sc_log.h"
#include"sc_lock.h"

#include"sc_math_common.h"

int buf_init(sc_buf_t *buf,int size)
{
	uint8_t *data=(uint8_t *)malloc(size);
	if(!data){
		sloge("alloc for buf data fail");
		return -1;
	}
	buf->data=data;
	buf->size=size;
	buf->level=0;
	buf->rd_ptr=buf->wr_ptr=buf->data;
	sc_lock_init(&buf->mutex,NULL);
	slogi("buf init ok");
	return 0;
}

int buf_reinit(sc_buf_t *buf){
	sc_lock(&buf->mutex);
	buf->level=0;
	buf->rd_ptr=buf->wr_ptr=buf->data;
	sc_unlock(&buf->mutex);
	return 0;
}

int buf_release(sc_buf_t * buf){
	sc_lock(&buf->mutex);
	/*看来主要就是维护由data指向首地址的一段size大小线性内存*/
	if(buf->data)
		free(buf->data);
	buf->size=0;
	sc_unlock(&buf->mutex);
	return 0;
}
//啥都没干啊!通过level来判断?
static int buf_empty(sc_buf_t * buf ){
	//no need to lock ,we will lock uplevel
	int ret= -1;
	if(buf->level  == 0 ) //空
	{
		ret=1;
	}
	else //非空
		ret= 0;
	return ret;
}

static int buf_full(sc_buf_t *buf){
//no need to lock ,we weill lock uplevel
	int ret=-1;
	if(buf->level == buf->size )
		ret=1;
	else
		ret = 0;
	return ret;
}
int buf_space(sc_buf_t *buf){
	int space=buf->size- buf->level;
	return space;
}
int buf_level(sc_buf_t *buf){
	int lev=buf->level;
	return lev;
}

	//执行写入到out，返回最多只能写入多少
	/*从线性内存中获取数据，写入out，size代表写入out了多少字节*/
int buf_get(sc_buf_t *buf,uint8_t *out,int size){
	sc_lock(&buf->mutex);
	int len=-1;
	len=buf_empty(buf);
	if(len == 1){//空
		len=0;
		goto EMPTY;//返回len为0
	}
	/*不能超过能力范围内*/
	len=MIN(buf->level,size);
	if(buf->wr_ptr > buf->rd_ptr)
	{
		memcpy(out,buf->rd_ptr,len);
		buf->rd_ptr+=len;
		buf->level -= len;
		goto OK;
	}else{//指针位置错开
		//直接复制
			if(len <= (int)(buf->data+buf->size - buf->rd_ptr))
			{
				memcpy(out,buf->rd_ptr,len);
				buf->rd_ptr += len;
				buf->level -= len;
				goto OK;
			}else //分开复制
			{
				int tail_len=(int)(buf->data+buf->size - buf->rd_ptr);
				memcpy(out,buf->rd_ptr,tail_len);
				//这里是不是直接就是让写指针指向头呢?
				buf->rd_ptr= buf->data+ len-tail_len /*buf->data 不是直接等于这个*/;
				memcpy(out+tail_len,buf->data,len-tail_len);

				buf->level -= len;
				goto OK;
			}
	}
	EMPTY:
	OK:
		sc_unlock(&buf->mutex);
	return len;
}

int buf_put(sc_buf_t *buf,uint8_t *in, int size){
	sc_lock(&buf->mutex);
	int len=buf_full(buf);
	if(len == 1 ){
		len = 0 ;
		goto FULL;
	}
	//计算剩余空间 并与 想写入的size比较
	len= MIN(buf->size - buf->level,size);
	if(buf->wr_ptr < buf->rd_ptr)
	{
		memcpy(buf->wr_ptr,in,len);
		buf->level += len;
		buf->wr_ptr += len;
		goto OK;
	}else{//两头写
//	先看尾巴是不是够用
		int tail_len=(int)(buf->data+buf->size - buf->wr_ptr);
		if(tail_len/*(int)(buf->data+buf->size - buf->wr_ptr)*/ >= len)
		{

			memcpy(buf->wr_ptr,in,len);
			buf->wr_ptr += len;
			buf->level += len;
			goto OK;
		}else{
			//int tail_len=(int)(buf->data+buf->size - buf->wr_ptr);
			//先写尾巴
			memcpy(buf->wr_ptr,in,tail_len);
			//再在头部写
			int ret_len= len- tail_len;
			memcpy(buf->data,in+tail_len,ret_len);
			buf->wr_ptr=buf->data+ret_len;
			buf->level += len;
			goto OK;
		}
   	}
	FULL:
	OK:
		sc_unlock(&buf->mutex);
		return len;
}

