#include "Buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <string.h>
#include <unistd.h>
#include <strings.h>
#include <sys/socket.h>

Buffer::Buffer(int size) : m_capacity(size)
{
	m_data = (char*)malloc(size);
	bzero(m_data, size);
}

Buffer::~Buffer()
{
	if (m_data != nullptr) {
		free(m_data);
	}
}

void Buffer::extendRoom(int size)
{
	//1.内存够用 - 不需要扩容
	if (writeableSize() >= size) {
		return;
	}
	//2.内存需要合并才够用 - 不需要扩容
	// 剩余的可写的内存 + 已读的内存 > size
	else if (m_readPos + writeableSize() >= size) {
		//得到未读的内存大小
		int readable = readableSize();
		//移动内存
		memcpy(m_data, m_data + m_readPos, readable);
		//更新位置
		m_readPos = 0;
		m_writePos = readable;
	}
	//3.内存不够用 - 扩容
	else {
		void* temp = realloc(m_data, m_capacity + size);
		if (temp == NULL) {
			return; //失败
		}
		memset((char*)temp + m_capacity, 0, size); //对扩展出部分数据初始化为0
		//更新数据
		m_data = static_cast<char*>(temp);
		m_capacity += size;
	}
}

int Buffer::appendData(const char* data, int size)
{

	if (data == nullptr || size <= 0) {
		return -1;
	}
	//试探性扩容（不一定扩容，函数内部有相关判定）
	extendRoom(size);
	//数据拷贝
	memcpy(m_data + m_writePos, data, size);
	m_writePos += size;
	return 0;
}

int Buffer::appendString(const char* data)
{
	int size = strlen(data);
	int ret = appendData( data, size);
	return ret;
}

int Buffer::appendString(const string data)
{
	int ret = appendString(data.data());
	return ret;
}

int Buffer::socketRead(int fd)
{
	// 接受数据函数:read/recv/readv (readv可指定多个数组进行数据接受）
	struct iovec vec[2];
	//初始化数组元素
	int writeable = writeableSize();
	vec[0].iov_base =m_data + m_writePos;
	vec[0].iov_len = writeable;
	char* tmpbuf = (char*)malloc(40960); //40K
	vec[1].iov_base = tmpbuf;
	vec[1].iov_len = 40960;
	int result = readv(fd, vec, 2);  // 返回接受的字节数
	if (result == -1) {
		return -1;
	}
	else if (result <= writeable) {
		//数据全部写入了vec[0], 即buffer->data
		m_writePos += result;
	}
	else {
		//数据有写入vec[1], 进行buffer的扩容与拷贝(buffer被写满）
		m_writePos = m_capacity;
		appendData(tmpbuf, result - writeable);
	}
	free(tmpbuf);
	return result;
}

char* Buffer::findCRLF()
{
	// strstr-- > 大字符串中匹配子字符串（遇到\0结束） 返回子字符串起始地址
	//memmem --> 大字符串中匹配数据块（需要指定数据块大小） 返回子字符串起始地址
	char* ptr = (char*)memmem(m_data + m_readPos, readableSize(), "\r\n", 2);
	return ptr;
}

int Buffer::sendData(int socket)
{
	// 判断有无数据
	int readable = readableSize(); //buff中未读的数据就是待发送的数据s
	if (readable > 0) {
		int count = send(socket, m_data + m_readPos, readable, MSG_NOSIGNAL); //忽略SIGPIPE信号（内核发出给到服务器进程，导致进程终止）
		if (count > 0) {
			m_readPos += count;
			usleep(1); //让接收端有时间处理
		}
		return count;
	}
	return 0;
}
