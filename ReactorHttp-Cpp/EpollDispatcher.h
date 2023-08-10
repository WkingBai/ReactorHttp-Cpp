#pragma once
#include <string>
#include <sys/epoll.h>
#include "Dispatcher.h"
#include "Channel.h"
#include "EventLoop.h"
using namespace std;

class EpollDispatcher : public Dispatcher
{
public:
	EpollDispatcher(EventLoop* evloop);
	virtual ~EpollDispatcher();
	//���
	int add() override;
	//ɾ��
	int remove()override;
	//�޸�
	int modify()override;
	//�¼���Ⲣ���ö�Ӧ�ص�����
	int dispatch(int timeout = 2) override; //��λ��s
private:
	int epollCtl(int op);
private:
	int m_epfd;
	struct epoll_event* m_events;
	const int m_maxNode = 520;
};