#pragma once
#pragma once
#include <string>
#include <poll.h>
#include "Dispatcher.h"
#include "Channel.h"
#include "EventLoop.h"
using namespace std;

class PollDispatcher : public Dispatcher
{
public:
	PollDispatcher(EventLoop* evloop);
	virtual ~PollDispatcher();
	//���
	int add() override;
	//ɾ��
	int remove()override;
	//�޸�
	int modify()override;
	//�¼���Ⲣ���ö�Ӧ�ص�����
	int dispatch(int timeout = 2) override; //��λ��s

private:
	int m_maxfd;
	struct pollfd *m_fds;
	const int m_maxNode = 1024;
};