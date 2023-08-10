#pragma once
#include <map>
#include <functional>
#include "Buffer.h"
using namespace std;

//定义状态码枚举
enum class StatusCode
{
	Unknown,
	OK = 200,
	MovePermanently = 301,
	MoveTemporarily = 302,
	BadRequest = 400,
	NotFound = 404
};
// 定义结构体
class HttpResponse
{
public:
	//可调用对象包装器（用来组织要回复给客户端的数据块）
	function<void(const string, Buffer*, int socket)> sendDataFunc;

	HttpResponse();
	~HttpResponse();
	//添加响应头
	void addHeader(const string key, const string value);
	//组织http响应数据
	void prepareMsg(Buffer* sendBuf, int socket);
	inline void setFileName(string name)
	{
		m_fileName = name;
	}
	inline void setStatusCode(StatusCode code)
	{
		m_statusCode = code;
	}
private:
	StatusCode m_statusCode;//状态行: 状态码， 状态描述
	string m_fileName; //文件名
	map<string, string> m_headers;//响应头 - 键值对
	//定义状态码和状态描述
	const map<int, string> m_info = {
		{200, "OK"},
		{301, "MovePermanently"},
		{302, "MoveTemporarily" },
		{400, "BadRequest"},
		{404, "NotFound"}
	};
};