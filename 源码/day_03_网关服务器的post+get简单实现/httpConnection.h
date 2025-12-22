//表示一次 HTTP 连接的会话对象（session)，负责网络与协议，只关心如何响应
/*
CServer 中的 acceptor 接受连接
	  ↓
创建 httpConnection（shared_ptr）
	  ↓
Start()
 ├─ async_read HTTP 请求
 └─ 启动超时定时器
	  ↓
handleReq()
	  ↓
writeResponse()
	  ↓
关闭连接 / 释放对象
*/

#pragma once
#include "const.h"

class httpConnection:public std::enable_shared_from_this<httpConnection>
{
public:
	friend class LogicSystem;
	httpConnection(tcp::socket socket);
	void Start();

private:
	void checkDeadline();//超时检测函数
	void writeResponse();//服务器回写应答函数
	void handleReq();//解析http请求，并生成对应的 HTTP 响应。
	void  PreParseGetParam();//解析url的函数,调用boost的库函数

	tcp::socket _socket;//通过这个已连接socket完成读写
	beast::flat_buffer _buffer{ 8192 };//接受字符的缓冲区，8k
	http::request<http::dynamic_body> _request;//请求的模板类，大小自动扩容
	http::response<http::dynamic_body> _response;//回复的模板类，大小自动扩容
	net::steady_timer _deadline//定时器，内联初始化
	{
		_socket.get_executor(),
		std::chrono::seconds(60)//60s未处理就报错
	};

	std::unordered_map<std::string, std::string> _get_query;//存储解析url得到的query的键值对
	std::string _get_path;//存储解析url得到path
};

