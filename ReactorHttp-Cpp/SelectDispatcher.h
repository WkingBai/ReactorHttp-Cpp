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
	//添加
	int add() override;
	//删除
	int remove()override;
	//修改
	int modify()override;
	//事件检测并调用对应回调函数
	int dispatch(int timeout = 2) override; //单位：s

private:
	fd_set m_readSet;
	fd_set m_writeSet;
	const int m_maxNode = 1024;
};