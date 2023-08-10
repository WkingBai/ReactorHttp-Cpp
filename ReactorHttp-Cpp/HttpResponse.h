#pragma once
#include <map>
#include <functional>
#include "Buffer.h"
using namespace std;

//����״̬��ö��
enum class StatusCode
{
	Unknown,
	OK = 200,
	MovePermanently = 301,
	MoveTemporarily = 302,
	BadRequest = 400,
	NotFound = 404
};
// ����ṹ��
class HttpResponse
{
public:
	//�ɵ��ö����װ����������֯Ҫ�ظ����ͻ��˵����ݿ飩
	function<void(const string, Buffer*, int socket)> sendDataFunc;

	HttpResponse();
	~HttpResponse();
	//�����Ӧͷ
	void addHeader(const string key, const string value);
	//��֯http��Ӧ����
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
	StatusCode m_statusCode;//״̬��: ״̬�룬 ״̬����
	string m_fileName; //�ļ���
	map<string, string> m_headers;//��Ӧͷ - ��ֵ��
	//����״̬���״̬����
	const map<int, string> m_info = {
		{200, "OK"},
		{301, "MovePermanently"},
		{302, "MoveTemporarily" },
		{400, "BadRequest"},
		{404, "NotFound"}
	};
};