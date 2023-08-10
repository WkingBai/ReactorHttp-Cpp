#pragma once
#include <thread>
#include <queue>
#include <map>
#include <mutex>
#include <string>
#include "Dispatcher.h"
using namespace std;

//处理该节点中channel的方式
enum class ElemType:char {ADD, DELETE, MODIFY};

//定义任务队列的节点
struct ChannelElement
{
	ElemType type; //如何处理该节点的channel
	Channel* channel;
};
//Dispatcher函数声明
class Dispatcher;
//反应堆模型
class EventLoop
{
public:
	EventLoop();
	EventLoop(const string threadName);
	~EventLoop(); //不操作（反应堆模型随服务器程序退出而销毁，服务器程序退出系统会自动回收其所占用的所有资源）
	//启动
	int run();
	//处理激活的文件描述符fd
	int eventActivate(int fd, int event);
	//添加任务到任务队列
	int addTask(Channel* channel, ElemType type);
	//处理任务队列中的任务
	int processTaskQ();
	//处理dispatcher中的节点
	int add(Channel* channel);
	int remove(Channel* channel);
	int modify(Channel* channel);
	//释放channel
	int freeChannel(Channel* channel);
	static int readLocalMessage(void* arg);
	int readMessage();
	//返回线程id
	inline thread::id getThreadID()
	{
		return m_threadID;
	}
	inline string getThreadName()
	{
		return m_ThreadName;
	}
private:
	void taskWakeup();

private:
	bool m_isQuit; //开关
	//该指针指向子类的实例 epoll,poll,select
	Dispatcher* m_dispatcher;
	//任务队列（链表实现）
	queue<ChannelElement*> m_taskQ;

	//文件描述符和channnel的映射关系
	map<int, Channel*> m_channelMap;
	//线程ID
	thread::id m_threadID;
	//线程名
	string m_ThreadName;
	//互斥锁
	mutex m_mutex;

	//用于解除阻塞
	int m_socketPair[2]; // 存储本地通信的fd 通过socketpair初始化
};