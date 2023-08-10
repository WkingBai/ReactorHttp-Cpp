#include "ThreadPool.h"
#include <assert.h>
#include <stdlib.h>
#include "Log.h"

ThreadPool::ThreadPool(EventLoop* mainLoop, int count)
{
	m_index = 0;
	m_isStart = false;
	m_mainLoop = mainLoop; //将主线程反应堆模型放入线程池
	m_threadNum = count;
	m_workerThreads.clear();
}

//(程序运行中不会被调用，可有可无）
ThreadPool::~ThreadPool()
{
	for (auto item : m_workerThreads) {
		delete item;
	}
}

void ThreadPool::run()
{
	assert(!m_isStart);
	if (m_mainLoop->getThreadID() != this_thread::get_id()) { //判断是否属于主线程
		exit(0);
	}
	m_isStart = true;
	if (m_threadNum > 0) { //判读线程池线程数量（为0代表没有子线程）
		for (int i = 0; i < m_threadNum; ++i) { //对线程池中子线程初始化
			WorkerThread* subThread = new WorkerThread(i);
			subThread->run();
			m_workerThreads.push_back(subThread);
		}
	}
}

EventLoop* ThreadPool::takeWorkerEventLoop()
{
	assert(m_isStart); //断言线程为运行
	if (m_mainLoop->getThreadID() != this_thread::get_id()) {  //判断调用此函数的是否是主线程
		exit(0);
	}
	//从线程池中找一个子线程，然后取出里边的反应堆实例
	EventLoop* evLoop = m_mainLoop;
	if (m_threadNum > 0) { //若子线程大于0
		evLoop = m_workerThreads[m_index]->getEventLoop();
		m_index = ++m_index % m_threadNum;
	}
	return evLoop;
}
