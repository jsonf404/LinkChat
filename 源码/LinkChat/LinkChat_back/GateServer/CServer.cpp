#include "CServer.h"
#include "httpConnection.h"
#include "AsioiocontextPool.h"

CServer::CServer(boost::asio::io_context& ioc, unsigned short& port)
	:_ioc(ioc),_acceptor(ioc,tcp::endpoint(tcp::v4(), port)){}

void CServer::Start()
{
	auto self = shared_from_this();//捕获self，延长CServer对象的声明周期， 直到异步回调结束

	//从连接池拿一个ioc，在此处构造AsioiocontextPool类，构造函数创建ioc池子。由于GetInstance是static线程安全，全局只会构造一次
	auto& io_context = AsioiocontextPool::GetInstance()->Getioc();

	//io_context作为参数初始化了httpConnection，且用 new_con 是一个指向 httpConnection 对象的智能指针对象
	std::shared_ptr<httpConnection> new_con = std::make_shared<httpConnection>(io_context);

	_acceptor.async_accept(new_con->GetSocket(), [self,new_con](beast::error_code ec) //异步回调，当有客户端连接到该端口时，把连接结果放入 _socket，然后调用回调
		{
			try 
			{
				if (ec)//出错了就放弃本次socket连接,继续监听下一个连接
				{
					self->Start();
					return;
				}

				//没错就创建新连接，并开启htppConnection类开始管理这个连接
				new_con->Start();//是httpconnection的start
				self->Start();
			}
			catch (std::exception& exp)
			{
				std::cout << "exception is " << exp.what() << std::endl;
				self->Start();
			}
		});
}