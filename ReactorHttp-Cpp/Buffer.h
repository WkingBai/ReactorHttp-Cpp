#pragma once
#include <string>
using namespace std;

class Buffer
{
public:
	Buffer(int size);
	~Buffer();
	//扩容
	void extendRoom(int size);
	//得到剩余的可写的内存容量
	int writeableSize()
	{
		return m_capacity - m_writePos;
	}
	//得到剩余的可读的内存容量
	int readableSize() 
	{
		return m_writePos - m_readPos;
	}
	//写内存 
	int appendData(const char* data, int size);
	int appendString(const char* data);
	int appendString(const string data);
	//接受套接字数据
	int socketRead(int fd); //返回接受字节数
	//根据\r\n取出一行，找出其在数据块中的位置，返回该位置
	char* findCRLF();
	//发送数据
	int sendData(int socket);
	//得到读数据的起始位置
	inline char* data()
	{
		return m_data + m_readPos;
	}

	inline int readPosIncrease(int count)
	{
		m_readPos += count;
		return m_readPos;
	}

private:
	//指向内存的指针
	char* m_data;
	//容量
	int m_capacity;
	//读位置
	int m_readPos = 0;
	//写位置
	int m_writePos = 0;
};