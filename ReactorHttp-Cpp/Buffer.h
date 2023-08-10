#pragma once
#include <string>
using namespace std;

class Buffer
{
public:
	Buffer(int size);
	~Buffer();
	//����
	void extendRoom(int size);
	//�õ�ʣ��Ŀ�д���ڴ�����
	int writeableSize()
	{
		return m_capacity - m_writePos;
	}
	//�õ�ʣ��Ŀɶ����ڴ�����
	int readableSize() 
	{
		return m_writePos - m_readPos;
	}
	//д�ڴ� 
	int appendData(const char* data, int size);
	int appendString(const char* data);
	int appendString(const string data);
	//�����׽�������
	int socketRead(int fd); //���ؽ����ֽ���
	//����\r\nȡ��һ�У��ҳ��������ݿ��е�λ�ã����ظ�λ��
	char* findCRLF();
	//��������
	int sendData(int socket);
	//�õ������ݵ���ʼλ��
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
	//ָ���ڴ��ָ��
	char* m_data;
	//����
	int m_capacity;
	//��λ��
	int m_readPos = 0;
	//дλ��
	int m_writePos = 0;
};