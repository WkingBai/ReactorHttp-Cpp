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
	//添加
	int add() override;
	//删除
	int remove()override;
	//修改
	int modify()override;
	//事件检测并调用对应回调函数
	int dispatch(int timeout = 2) override; //单位：s
private:
	int epollCtl(int op);
private:
	int m_epfd;
	struct epoll_event* m_events;
	const int m_maxNode = 520;
};