/*
CServer 是监听器（Listener / Acceptor），职责非常明确：
1. 监听端口
2. 接收 TCP 连接
3. 把“已连接的 socket”交给 httpConnection

CServer 不处理 HTTP 协议本身，HTTP 逻辑全部在 httpConnection 中。
这是 Boost.Beast 推荐的 Server / Session 分离模型。
*/

#pragma once
#include "const.h"

class CServer:public std::enable_shared_from_this<CServer>
{
public:
	CServer(boost::asio::io_context& ioc, unsigned short& port);//异步上下文ioc，端口号
	void Start();

private:
	tcp::acceptor _acceptor;//服务器接受请求的对象,同时完成open、bind、listen
	net::io_context& _ioc;//所有异步操作最终都由它调度
};

