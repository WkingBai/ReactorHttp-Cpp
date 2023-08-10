#include "TcpConnection.h"
#include <stdlib.h>
#include "HttpRequest.h"
#include <stdio.h>
#include "Log.h"

//evLoop 在同一子线程处理， processRead执行完毕后，在下次（epoll)检测到读事件后才会触发processWrite,不是同时处理的。
//当发送大文件时，WriteBuf中内存不够就难以发送
int TcpConnection::processRead(void* arg)
{
	TcpConnection* conn = static_cast<TcpConnection*>(arg);
	//接受数据
	int socket = conn->m_channel->getSocket();
	int count = conn->m_readBuf->socketRead(socket);
	Debug("接受到的http请求数据:%s", conn->m_readBuf->data());
	if (count > 0) {
		//接收到了http请求， 解析http请求
#ifdef MSG_SEND_AUTO
		conn->m_channel->writeEventEnable(true); //添加写事件
		conn->m_evLoop->addTask(conn->m_channel, ElemType::MODIFY); //检测事件变为读写
#endif
		bool flag = conn->m_request->parseRequest(conn->m_readBuf, conn->m_response, conn->m_writeBuf, socket);
		if (!flag) {
			//解析失败，回复一个简单的html
			string errMsg = "Http/1.1 400 Bad Request\r\n\r\n";
			conn->m_writeBuf->appendString(errMsg);
		}
	}
	else {  //(count == 0)
#ifdef MSG_SEND_AUTO
		conn->m_evLoop->addTask(conn->m_channel, ElemType::DELETE);
#endif
	}
	//断开连接
#ifndef MSG_SEND_AUTO
	conn->m_evLoop->addTask(conn->m_channel, ElemType::DELETE);
#endif
	return 0;
}

int TcpConnection::processWrite(void* arg)
{
	Debug("开始发送数据了(基于写事件发送)...");
	TcpConnection* conn = static_cast<TcpConnection*>(arg);
	//发送数据
	int count = conn->m_writeBuf->sendData(conn->m_channel->getSocket());
	if (count > 0) {
		//判断数据是否被全部发送出去
		if (conn->m_writeBuf->readableSize() == 0) {
			//1.不再检测写事件 -- 修改channel中保存的事件
			conn->m_channel->writeEventEnable(false);
 			//2. 修改dispatcher检测的集合 -- 添加任务节点(原本检测读写事件，修改未只检测读事件）
			conn->m_evLoop->addTask(conn->m_channel, ElemType::MODIFY);
			//3. 删除节点 (1,2可不加）
			conn->m_evLoop->addTask(conn->m_channel, ElemType::DELETE);
		}
	}
	return 0;
}

int TcpConnection::destroy(void* arg)
{
	
	TcpConnection* conn = static_cast<TcpConnection*>(arg);
	if (conn != nullptr)
	{
		delete conn;
	}
	return 0;
}

TcpConnection::TcpConnection(int fd, EventLoop* evLoop)
{
	m_evLoop = evLoop;
	m_readBuf = new Buffer(10240);
	m_writeBuf = new Buffer(10240);
	//http
	m_request = new HttpRequest;
	m_response = new HttpResponse;
	m_name = "Connection-" + to_string(fd);
	m_channel = new Channel(fd, FDEvent::ReadEvent, processRead, processWrite, destroy, this);
	m_evLoop->addTask(m_channel, ElemType::ADD);
	Debug("和客户端建立连接， threadName:%s, threadID:%ld, connName:%s", m_evLoop->getThreadName().data(), m_evLoop->getThreadID(), m_name.data());
}

TcpConnection::~TcpConnection()
{
	if (m_readBuf && m_readBuf->readableSize() == 0 &&
		m_writeBuf && m_writeBuf->readableSize() == 0) {
		delete m_readBuf;
		delete m_writeBuf;
		delete m_request;
		delete m_response;
		m_evLoop->freeChannel(m_channel);//释放channel
		Debug("连接断开，释放资源, gameover, connName:%s", m_name.data());
	}
	
}
