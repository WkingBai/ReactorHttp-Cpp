#include "Dispatcher.h"
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include "SelectDispatcher.h"

SelectDispatcher::SelectDispatcher(EventLoop* evloop) : Dispatcher(evloop)
{
	FD_ZERO(&m_readSet);
	FD_ZERO(&m_writeSet);
	m_name = "Select";
}

SelectDispatcher::~SelectDispatcher()
{
}

int SelectDispatcher::add()
{
	if (m_channel->getSocket() >= m_maxNode) {
		return -1;
	}
	if (m_channel->getEvent() & (int)FDEvent::ReadEvent) {
		FD_SET(m_channel->getSocket(), &m_readSet);
	}
	if (m_channel->getEvent() & (int)FDEvent::WriteEvent) {
		FD_SET(m_channel->getSocket(), &m_writeSet);
	}
	return 0;
}

int SelectDispatcher::remove()
{
	if (m_channel->getEvent() & (int)FDEvent::ReadEvent) {
		FD_CLR(m_channel->getSocket(), &m_readSet);
	}
	if (m_channel->getEvent() & (int)FDEvent::WriteEvent) {
		FD_CLR(m_channel->getSocket(), &m_writeSet);
	}
	// 通过channel 释放对应的 TcpConnection 资源
	m_channel->destroyCallback(const_cast<void*>(m_channel->getArg()));
	return 0;
}

int SelectDispatcher::modify()
{
	if (m_channel->getEvent() & (int)FDEvent::ReadEvent) {
		FD_SET(m_channel->getSocket(), &m_readSet);
		FD_CLR(m_channel->getSocket(), &m_writeSet);
	}
	if (m_channel->getEvent() & (int)FDEvent::WriteEvent) {
		FD_SET(m_channel->getSocket(), &m_writeSet);
		FD_CLR(m_channel->getSocket(), &m_readSet);
	}
	return 0;
}

int SelectDispatcher::dispatch(int timeout)
{
	struct timeval val; //超时时间的结构体
	val.tv_sec = timeout;
	val.tv_usec = 0; //必须指定
	//select 调用完成会改变readSet及writeSet的数据，所以需要对原始数据进行备份
	fd_set rdtmp = m_readSet;
	fd_set wrtmp = m_writeSet;
	int count = select(m_maxNode, &rdtmp, &wrtmp, NULL, &val); //函数中单位为:ms
	if (count == -1) {
		perror("Select");
		exit(0);
	}
	for (int i = 0; i < m_maxNode; ++i) {
		if (FD_ISSET(i, &rdtmp))
		{
			m_evLoop->eventActivate(i, (int)FDEvent::ReadEvent);
		}
		if (FD_ISSET(i, &wrtmp)) {
			m_evLoop->eventActivate(i, (int)FDEvent::WriteEvent);
		}
	}
	return 0;
}
