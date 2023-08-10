#pragma once
#include <string>
#include "Channel.h"
#include "EventLoop.h"
using namespace std;

class EventLoop; //函数声明
class Dispatcher
{
public:
	Dispatcher(EventLoop* evloop);
	virtual ~Dispatcher();
	//添加
	virtual int add();
	//删除
	virtual int remove();
	//修改
	virtual int modify();
	//事件检测并调用对应回调函数
	virtual int dispatch(int timeout = 2); //单位：s
	inline void setChannel(Channel* channel)
	{
		m_channel = channel;
	}
protected: //方便子类继承（private不可被继承）
 	string m_name = string();
	Channel* m_channel;
	EventLoop* m_evLoop;

};