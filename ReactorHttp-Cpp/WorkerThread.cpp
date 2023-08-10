#include "WorkerThread.h"
#include <string>
#include <stdio.h>

WorkerThread::WorkerThread(int index)
{
	m_evLoop = nullptr;
	m_thread = nullptr;
	m_threadID = thread::id();
	m_name = "SubThread-" + to_string(index);
}

//一直存在线程池中（程序运行中不会被调用，可有可无）
WorkerThread::~WorkerThread()
{
	if (m_thread != nullptr) {
		delete m_thread;
	}
}

void WorkerThread::run()
{
	//创建子线程
	m_thread = new thread(&WorkerThread::running, this);
	//阻塞主线程，让当前函数不会直接结束，等待子线程的反应堆模型创建完毕 （线程间随机抢占cpu，无法确定那个先完成任务）
	unique_lock<mutex> locker(m_mutex); 
	while (m_evLoop == nullptr) //子线程创建成功 evLoop（共享资源） 不为空
	{
		m_cond.wait(locker);
	}
}

// 子线程的回调函数
void WorkerThread::running()
{
	m_mutex.lock();
	m_evLoop = new EventLoop(m_name);
	m_mutex.unlock();
	m_cond.notify_one(); //唤醒条件阻塞在条件变量上的某一个线程(唤醒主线程）
	m_evLoop->run();
}
