#pragma once
#include "Buffer.h"
#include "HttpResponse.h"
#include <stdbool.h>
#include <map>
using namespace std;

//当前的解析状态
enum class PrecessState : char
{
	ParseReqLine,
	ParseReqHeaders,
	ParseReqBody,
	ParseReqDone
};
//定义http请求的结构体
class HttpRequest
{
public:
	HttpRequest();
	~HttpRequest();
	//重置结构体
	void reset();
	//添加请求头
	void addHeader(const string key, const string value);
	//根据key得到请求头的value
	string getHeader(const string key);
	//解析请求行
	bool parseRequestLine(Buffer* readBuf);
	//解析请求头
	bool parseRequestHeader(Buffer* readBuf);
	//解析http请求协议
	bool parseRequest(Buffer* readBuf, HttpResponse* response, Buffer* sendBuf, int socket);
	//处理http请求协议
	bool processRequest(HttpResponse* response);
	string decodeMsg(string msg);
	const string getFileType(const string name);
	static void sendDir(string dirName, Buffer* sendBuf, int cfd);
	static void sendFile(string fileName, Buffer* sendBuf, int cfd);
	inline void setMethod(string method)
	{
		m_method = method;
	}
	inline void setUrl(string url)
	{
		m_url = url;
	}
	inline void setVersion(string version)
	{
		m_version = version;
	}
	//获取处理状态
	inline PrecessState getState()
	{
		return m_curState;
	}
	inline void setSate(PrecessState state)
	{
		m_curState = state;
	}
private:
	//解码函数
	char* splitRequestLine(const char* start, const char* end, const char* sub, function<void(string)> callback);
	int hexToDec(char c);
	

private:
	string m_method; // 请求方法（get，post)
	string m_url;   // 请求资源
	string m_version; //http版本
	map<string, string> m_reqHeaders; //键值对数组
	PrecessState m_curState;
};
