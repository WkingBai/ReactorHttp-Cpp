//#define _GNU_SOURCE  //memmem 的一个头部宏定义
#include "HttpRequest.h"
#include "TcpConnection.h"
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>

#define HeaderSize 12

HttpRequest::HttpRequest()
{
	reset();
}

HttpRequest::~HttpRequest()
{
}

void HttpRequest::reset()
{
	m_curState = PrecessState::ParseReqLine;
	m_method = m_url = m_version = string();
	m_reqHeaders.clear();
}

void HttpRequest::addHeader(const string key, const string value)
{
	if (key.empty() || value.empty()) {
		return;
	}
	m_reqHeaders.insert(make_pair(key, value));
}

string HttpRequest::getHeader(const string key)
{
	auto item = m_reqHeaders.find(key);
	if (item == m_reqHeaders.end()) {
		return string();
	}
	return item->second;
}

bool HttpRequest::parseRequestLine(Buffer* readBuf)
{
	// 读出请求行
	// 保存字符串起始地址
	char* start = readBuf->data();
	// 保存字符串结束地址
	char* end = readBuf->findCRLF();
	//请求行的长度
	int lineSize = end - start;

	if (lineSize > 0) {
		auto methodFunc = bind(&HttpRequest::setMethod, this, placeholders::_1);
		start = splitRequestLine(start, end, " ", methodFunc); //请求方式
		auto urlFunc = bind(&HttpRequest::setUrl, this, placeholders::_1);
		start = splitRequestLine(start, end, " ", urlFunc);//请求的静态资源
		auto versionFunc = bind(&HttpRequest::setVersion, this, placeholders::_1);
		splitRequestLine(start, end, nullptr, versionFunc);//http 版本
		//为解析请求头做准备
		readBuf->readPosIncrease(lineSize + 2); // 加2跳过/r/n,到达下一行第一个字符
		//修改状态
		setSate(PrecessState::ParseReqHeaders);
		return true;
	}
	return false;
}

bool HttpRequest::parseRequestHeader(Buffer* readBuf)
{
	char* end = readBuf->findCRLF();
	if (end != nullptr) {
		char* start = readBuf->data();
		int lineSize = end - start;
		//基于": "搜索字符串
		char* middle = static_cast<char*>(memmem(start, lineSize, ": ", 2));
		if (middle != nullptr) {
			int keyLen = middle - start;
			int valueLen = end - middle - 2;
			if (keyLen > 0 && valueLen > 0) {
				string key(start, keyLen);
				string value(middle + 2, valueLen);
				addHeader(key, value);
			}

			//为解析下一行准备
			readBuf->readPosIncrease(lineSize + 2); // 加2跳过/r/n,到达下一行第一个字符
		}
		else {
			//请求头已经被解析完了,跳过空行(\r\n)
			readBuf->readPosIncrease(2);
			//修改解析状态
			//忽略post请求，按照 get 请求处理
			setSate(PrecessState::ParseReqDone);
		}
		return true;
	}
	return false;
}

bool HttpRequest::parseRequest(Buffer* readBuf, HttpResponse* response, Buffer* sendBuf, int socket)
{
	bool flag = true;
	while (m_curState != PrecessState::ParseReqDone) {
		switch (m_curState) {
		case PrecessState::ParseReqLine:
			flag = parseRequestLine(readBuf);
			break;
		case PrecessState::ParseReqHeaders:
			flag = parseRequestHeader(readBuf);
			break;
		case PrecessState::ParseReqBody:
			break;
		default:
			break;
		}
		if (!flag) {
			return flag;
		}
		//判断释放解析完毕了， 如果解析完毕，需要准备回复的数据
		if (m_curState == PrecessState::ParseReqDone) {
			//1. 根据解析出的原始数据， 对客户端的请求做出处理
			processRequest(response);
			//2. 组织响应数据并发给客户端
			response->prepareMsg(sendBuf, socket);

		}
	}
	//还原req->curState状态，使得下次解析请求正常
	m_curState = PrecessState::ParseReqLine;
	return flag;
}

bool HttpRequest::processRequest(HttpResponse* response)
{
	if (strcasecmp(m_method.data(), "get") != 0) {
		return -1;
	}
	m_url = decodeMsg(m_url); //解码
	//存储客户端请求的静态资源的相对路径（目录或文件）
	const char* file = NULL;
	if (strcmp(m_url.data(), "/") == 0)
	{
		file = "./";
	}
	else {
		file = m_url.data() + 1;
	}
	//获取文件属性
	struct stat st;
	int ret = stat(file, &st);
	if (ret == -1) {
		//文件不存在 -- 回复404
		response->setFileName("404.html");
		response->setStatusCode(StatusCode::NotFound);
		//响应头
		response->addHeader("Content-type", getFileType(".html"));
		response->sendDataFunc = sendFile; 
		return 0;
	}

	response->setFileName(file);
	response->setStatusCode(StatusCode::OK);
	//判断文件类型
	if (S_ISDIR(st.st_mode)) {
		//把这个目录中的内容发送给客户端
		//响应头
		response->addHeader("Content-type", getFileType(".html"));
		response->sendDataFunc = sendDir;

	}
	else {
		//把文件的内容发送给客户端
		//响应头
		response->addHeader("Content-type", getFileType(file));
		response->addHeader("Content-length", to_string(st.st_size));
		response->sendDataFunc = sendFile;
	}
	return false;
}

char* HttpRequest::splitRequestLine(const char* start, const char* end, const char* sub, function<void(string)> callback)
{
	char* space = const_cast<char*>(end);
	if (sub != NULL) {
		space = static_cast<char*>(memmem(start, end - start, sub, strlen(sub)));
		assert(space != NULL);
	}
	int length = space - start;
	callback(string(start, length));
	return space + 1; // 返回解析到地址的下一位置
}

string HttpRequest::decodeMsg(string msg)
{
	const char* from = msg.data();
	string str = string();
	for (; *from != '\0'; ++from) {
		//isxdigit -> 判断字符是不是16进制格式， 取值在0-f
		if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2])) {
			//将一个16进制的数 -> 十进制 将这个数值赋值给了字符 int -> char
			// B2 = 178
			//将3个字符，变成了一个字符， 这个字符就是原始数据
			str.append(1, hexToDec(from[1]) * 16 + hexToDec(from[2]));

			//跳过from[1] 和 from[2] 因此在当前循环中已经处理过了
			from += 2;
		}
		else {
			//字符拷贝，赋值
			str.append(1, *from);
		}
	}
	str.append(1, '\0');
	return str;
}

//将字符转换为整形
int HttpRequest::hexToDec(char c)
{
	
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	return 0;
}

const string HttpRequest::getFileType(const string name)
{
	//自右向左查找'.'字符，如果不存在返回NULL
	const char* dot = strrchr(name.data(), '.');
	if (dot == NULL)
		return "text/plain; charset=utf-8"; //纯文本
	if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
		return "text/html; charset=utf-8";
	if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
		return "image/jpeg";
	if (strcmp(dot, ".gif") == 0)
		return "image/gif";
	if (strcmp(dot, ".png") == 0)
		return "image/png";
	if (strcmp(dot, ".css") == 0)
		return "text/css";
	if (strcmp(dot, ".au") == 0)
		return "audio/basic";
	if (strcmp(dot, ".wav") == 0)
		return "audio/wav";
	if (strcmp(dot, ".avi") == 0)
		return "video/x-msvideo";
	if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
		return "video/quicktime";
	if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
		return "video/mpeg";
	if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
		return "video/vrml";
	if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
		return "video/midi";
	if (strcmp(dot, ".mp3") == 0)
		return "audio/mpeg";
	if (strcmp(dot, ".ogg") == 0)
		return "application/ogg";
	if (strcmp(dot, ".pac") == 0)
		return "application/x-ns-proxy-autoconfig";

	return "text/plain; charset=utf-8";
}

void HttpRequest::sendDir(string dirName, Buffer* sendBuf, int cfd)
{
	char buf[4096] = { 0 };
	sprintf(buf, "<html><head><title>%s</title></head><body><table>", dirName.data());
	struct dirent** namelist;
	int num = scandir(dirName.data(), &namelist, NULL, alphasort);
	for (int i = 0; i < num; ++i) {
		//取出文件名字(namelist 指向的是一个指针数组）
		char* name = namelist[i]->d_name;
		struct stat st;
		char subPath[1024] = { 0 };
		sprintf(subPath, "%s/%s", dirName.data(), name);
		stat(subPath, &st);
		if (S_ISDIR(st.st_mode)) {
			//如果是目录
			//a 标签 <a href="">name</a>  通过href属性实现超链接
			sprintf(buf + strlen(buf),
				"<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>",
				name, name, st.st_size);

		}
		else {
			//如果是文件
			sprintf(buf + strlen(buf),
				"<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",
				name, name, st.st_size);
		}
		sendBuf->appendString(buf);
#ifndef MSG_SEND_AUTO
		sendBuf->sendData(cfd); //发送数据
#endif
		memset(buf, 0, sizeof(buf)); //清空buf
		free(namelist[i]); //释放内存
	}
	sprintf(buf, "</table></body></html>");
	sendBuf->appendString(buf);

#ifndef MSG_SEND_AUTO
	sendBuf->sendData(cfd); //发送数据
#endif

	free(namelist); //释放内存
}

void HttpRequest::sendFile(const string fileName, Buffer* sendBuf, int cfd)
{
	//1.打开文件
	int fd = open(fileName.data(), O_RDONLY);
	assert(fd > 0);
	while (1) {
		char buf[1024];
		int len = read(fd, buf, sizeof buf);
		if (len > 0) {
			sendBuf->appendData(buf, len); //buf最后一个字符不一定为\0，所以不能使用appendString
#ifndef MSG_SEND_AUTO
			sendBuf->sendData(cfd); //发送数据
#endif
		}
		else if (len == 0) {
			break;
		}
		else {
			close(fd);
			perror("read");
		}
	}
	close(fd);
}
