#pragma once
#include "EventLoop.h"
#include "ThreadPool.h"

class TcpServer
{
public:
	TcpServer(unsigned short port, int threadNum);
	~TcpServer();
	//���ü���
	void setListen();
	//����������
	void run();
	static int acceptConnection(void* arg);

private:
	int m_threadNum;
	int m_lfd;
	unsigned short m_port;
	EventLoop* m_mainLoop;
	ThreadPool* m_threadPool;
};