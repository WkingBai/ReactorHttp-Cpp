#pragma once
#pragma once
#pragma once
#include <string>
#include <sys/select.h>
#include "Dispatcher.h"
#include "Channel.h"
#include "EventLoop.h"
using namespace std;

class SelectDispatcher : public Dispatcher
{
public:
	SelectDispatcher(EventLoop* evloop);
	virtual ~SelectDispatcher();
	//���
	int add() override;
	//ɾ��
	int remove()override;
	//�޸�
	int modify()override;
	//�¼���Ⲣ���ö�Ӧ�ص�����
	int dispatch(int timeout = 2) override; //��λ��s

private:
	fd_set m_readSet;
	fd_set m_writeSet;
	const int m_maxNode = 1024;
};