#pragma once
#include <vector>
#include <stdbool.h>
#include "WorkerThread.h"
#include "EventLoop.h"
using namespace std;

//定义线程池
class ThreadPool
{
public:
	ThreadPool(EventLoop* mainLoop, int count);
	~ThreadPool();
	//启动线程池
	void run();
	//取出线程池中的某个子线程的反应堆实例(将任务放入其中， 主线程操作）
	EventLoop* takeWorkerEventLoop();
private:
	//主线程的反应堆模型
	EventLoop* m_mainLoop;
	bool m_isStart; //开关
	int m_threadNum; //线程个数
	vector<WorkerThread*> m_workerThreads; //线程队列
	int m_index; //第几个线程
};