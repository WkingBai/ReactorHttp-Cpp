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
	//添加
	int add() override;
	//删除
	int remove()override;
	//修改
	int modify()override;
	//事件检测并调用对应回调函数
	int dispatch(int timeout = 2) override; //单位：s

private:
	int m_maxfd;
	struct pollfd *m_fds;
	const int m_maxNode = 1024;
};