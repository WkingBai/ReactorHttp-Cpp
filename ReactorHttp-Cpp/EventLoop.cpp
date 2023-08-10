﻿#include <assert.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "EventLoop.h"
#include "EpollDispatcher.h"
#include "PollDispatcher.h"
#include "SelectDispatcher.h"

//委托构造函数（注意不要形成闭环）
EventLoop::EventLoop() : EventLoop(string())
{
}

EventLoop::EventLoop(const string threadName)
{
	m_isQuit = true; //默认没有启动
	m_threadID = this_thread::get_id(); 
	 m_ThreadName == string() ? "MainThread" : threadName;
	m_dispatcher = new EpollDispatcher(this); //使用epoll模型
	m_channelMap.clear();
	int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, m_socketPair); //用于创建一对无名的，相互连接的套接字
	if (ret == -1)
	{
		perror("socketpair");
		exit(0);
	}
//指定规则：evLoop->socketPair[0] 发送数据， evLoop->socketPair[1] 接受数据 (跟随evLoop，不需要释放）
//function可调用对象包装器不能直接打包类内的非静态成员函数
#if 0
	//1.readLocalMessage改为静态成员函数
	Channel* channel = new Channel(m_socketPair[1], FDEvent::ReadEvent, readLocalMessage, nullptr, nullptr, this);
#else
	//2.可调用对象绑定器 - bind
	auto obj = bind(&EventLoop::readMessage, this);
	Channel* channel = new Channel(m_socketPair[1], FDEvent::ReadEvent, obj, nullptr, nullptr, this);
#endif
	// channel 添加到任务队列
	addTask(channel, ElemType::ADD);
}

EventLoop::~EventLoop()
{
}

int EventLoop::run()
{
	m_isQuit = false;
	//比较线程ID是否正常
	if (m_threadID != this_thread::get_id()) {
		return -1;
	}
	//循环进行事件处理
	while (!m_isQuit) {
		m_dispatcher->dispatch();
		processTaskQ();
	}
	return 0;
}

int EventLoop::eventActivate(int fd, int event)
{

	if (fd < 0 ) {
		return -1;
	}
	//取出channel
	Channel* channel = m_channelMap[fd];
	assert(channel->getSocket() == fd); //断言channel->fd与参数fd一致
	//判断事件类型
	if (event & (int)FDEvent::ReadEvent && channel->readCallback) {
		channel->readCallback(const_cast<void*>(channel->getArg()));
	}
	if (event & (int)FDEvent::WriteEvent && channel->writeCallback) {

		channel->writeCallback(const_cast<void*>(channel->getArg()));
	}
	return 0;
}

int EventLoop::addTask(Channel* channel, ElemType type)
{
	//线程同步，加锁，保护共享资源
	m_mutex.lock();
	//创建新节点
	ChannelElement* node = new ChannelElement;
	node->channel = channel;
	node->type = type;
	m_taskQ.push(node);//添加节点到队列
	m_mutex.unlock();
	// 处理节点
	/*
	* 细节：
	* 1.对于链表节点的添加:可能是当前线程也可能是其他线程（主线程）
	*	1）.修改fd的事件，当前子线程发起，当前子线程处理
	*	2）.添加新的fd， 添加任务节点的操作是主线程发起的
	* 2. 不能让主线程处理任务队列，需要由当前的子线程去处理
	*/
	if (m_threadID == this_thread::get_id()) {
		// 当前子线程(基于子线程的角度分析)
		processTaskQ();//处理任务列表中任务
	}
	else {
		//当前主线程 -- 告诉子线程处理任务队列中任务
		//1. 子线程正在工作 2.子线程被阻塞了（dispatcher): select, poll, epoll (timeout = 2s) 检测的集合里没有激活的文件描述符fd
		taskWakeup(); //发送数据唤醒子线程（触发子线程读事件）
	}
	return 0;
}

int EventLoop::processTaskQ()
{
	while (!m_taskQ.empty()) {
		m_mutex.lock();
		ChannelElement* node = m_taskQ.front();
		m_taskQ.pop();  //删除节点
		m_mutex.unlock();
		Channel* channel = node->channel;
		if (node->type == ElemType::ADD) {
			//添加
			add(channel);
		}
		else if (node->type == ElemType::DELETE) {
			//删除
			remove(channel);
		}
		else if (node->type == ElemType::MODIFY) {
			//修改
			modify(channel);
		}
		delete node; // 释放已处理的任务
	}
	
	return 0;
}

int EventLoop::add(Channel* channel)
{
	int fd = channel->getSocket();
	//找到fd对应数组元素位置，并存储
	if (m_channelMap.find(fd) == m_channelMap.end()) {
		m_channelMap.insert(make_pair(fd,channel));
		//将channel->fd添加到文件描述符待检测集合中（epoll, poll, select);
		m_dispatcher->setChannel(channel); 
		int ret = m_dispatcher->add();
		return ret;
	}
	return -1;
}

int EventLoop::remove(Channel* channel)
{
	int fd = channel->getSocket();
	if (m_channelMap.find(fd) == m_channelMap.end()) {
		return -1;
	}
	m_dispatcher->setChannel(channel);
	int ret = m_dispatcher->remove(); //从文件描述符检测集合中删除
	return ret;
}

int EventLoop::modify(Channel* channel)
{
	int fd = channel->getSocket();
	if (m_channelMap.find(fd) == m_channelMap.end()) {	
		return -1;
	}
	m_dispatcher->setChannel(channel);
	int ret = m_dispatcher->modify();
	return ret;
}

int EventLoop::freeChannel(Channel* channel)
{
	// 删除 channel 和 fd 的对应关系
	auto it = m_channelMap.find(channel->getSocket());
	if (it != m_channelMap.end())
	{
		m_channelMap.erase(it);
		close(channel->getSocket());
		delete channel;
	}
	return 0;
}

int EventLoop::readLocalMessage(void* arg)
{
	EventLoop* evLoop = static_cast<EventLoop*>(arg);
	char buf[256];
	read(evLoop->m_socketPair[1], buf, sizeof(buf));
	return 0;
}

int EventLoop::readMessage()
{
	char buf[256];
	read(m_socketPair[1], buf, sizeof(buf));
	return 0;
}

void EventLoop::taskWakeup()
{
	const char* msg = "唤醒阻塞程序！";
	write(m_socketPair[0], msg, strlen(msg));
}
