#pragma once
#include "Buffer.h"
#include "HttpResponse.h"
#include <stdbool.h>
#include <map>
using namespace std;

//��ǰ�Ľ���״̬
enum class PrecessState : char
{
	ParseReqLine,
	ParseReqHeaders,
	ParseReqBody,
	ParseReqDone
};
//����http����Ľṹ��
class HttpRequest
{
public:
	HttpRequest();
	~HttpRequest();
	//���ýṹ��
	void reset();
	//�������ͷ
	void addHeader(const string key, const string value);
	//����key�õ�����ͷ��value
	string getHeader(const string key);
	//����������
	bool parseRequestLine(Buffer* readBuf);
	//��������ͷ
	bool parseRequestHeader(Buffer* readBuf);
	//����http����Э��
	bool parseRequest(Buffer* readBuf, HttpResponse* response, Buffer* sendBuf, int socket);
	//����http����Э��
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
	//��ȡ����״̬
	inline PrecessState getState()
	{
		return m_curState;
	}
	inline void setSate(PrecessState state)
	{
		m_curState = state;
	}
private:
	//���뺯��
	char* splitRequestLine(const char* start, const char* end, const char* sub, function<void(string)> callback);
	int hexToDec(char c);
	

private:
	string m_method; // ���󷽷���get��post)
	string m_url;   // ������Դ
	string m_version; //http�汾
	map<string, string> m_reqHeaders; //��ֵ������
	PrecessState m_curState;
};
