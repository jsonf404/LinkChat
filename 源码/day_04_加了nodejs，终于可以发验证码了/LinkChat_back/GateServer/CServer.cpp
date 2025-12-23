#include "CServer.h"
#include "httpConnection.h"

CServer::CServer(boost::asio::io_context& ioc, unsigned short& port)
	:_ioc(ioc),_acceptor(ioc,tcp::endpoint(tcp::v4(), port)), _socket(ioc)//
{

}

void CServer::Start()
{
	auto self = shared_from_this();//捕获self，延长CServer对象的声明周期，知道异步回调结束
	//异步回调，当有客户端连接到该端口时，把连接结果放入 _socket，然后调用回调
	_acceptor.async_accept(_socket, [self](beast::error_code ec) 
		{
			try 
			{
				//出错了就放弃本次socket连接,继续监听下一个连接
				if (ec)
				{
					self->Start();
					return;
				}

				//没错就创建新连接，并开启htppConnection类开始管理这个连接
				std::make_shared<httpConnection>(std::move(self->_socket))->Start();

				self->Start();
			}
			catch (std::exception& exp)
			{
				std::cout << "exception is " << exp.what() << std::endl;
				self->Start();
			}
		});
}