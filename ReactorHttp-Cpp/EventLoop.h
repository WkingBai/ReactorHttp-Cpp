#pragma once
#include <thread>
#include <queue>
#include <map>
#include <mutex>
#include <string>
#include "Dispatcher.h"
using namespace std;

//����ýڵ���channel�ķ�ʽ
enum class ElemType:char {ADD, DELETE, MODIFY};

//����������еĽڵ�
struct ChannelElement
{
	ElemType type; //��δ���ýڵ��channel
	Channel* channel;
};
//Dispatcher��������
class Dispatcher;
//��Ӧ��ģ��
class EventLoop
{
public:
	EventLoop();
	EventLoop(const string threadName);
	~EventLoop(); //����������Ӧ��ģ��������������˳������٣������������˳�ϵͳ���Զ���������ռ�õ�������Դ��
	//����
	int run();
	//��������ļ�������fd
	int eventActivate(int fd, int event);
	//��������������
	int addTask(Channel* channel, ElemType type);
	//������������е�����
	int processTaskQ();
	//����dispatcher�еĽڵ�
	int add(Channel* channel);
	int remove(Channel* channel);
	int modify(Channel* channel);
	//�ͷ�channel
	int freeChannel(Channel* channel);
	static int readLocalMessage(void* arg);
	int readMessage();
	//�����߳�id
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
	bool m_isQuit; //����
	//��ָ��ָ�������ʵ�� epoll,poll,select
	Dispatcher* m_dispatcher;
	//������У�����ʵ�֣�
	queue<ChannelElement*> m_taskQ;

	//�ļ���������channnel��ӳ���ϵ
	map<int, Channel*> m_channelMap;
	//�߳�ID
	thread::id m_threadID;
	//�߳���
	string m_ThreadName;
	//������
	mutex m_mutex;

	//���ڽ������
	int m_socketPair[2]; // �洢����ͨ�ŵ�fd ͨ��socketpair��ʼ��
};