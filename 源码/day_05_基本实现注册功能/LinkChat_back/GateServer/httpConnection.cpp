//连接是否成功的处理逻辑在LogicSystem，httpConnection只关心如何响应
/*
Start()
 └─ async_read
	 └─ HTTP 请求到达
		 ├─ handleReq()
		 │    └─ writeResponse()
		 │         └─ async_write
		 │              ├─ shutdown send
		 │              └─ cancel deadline
		 └─ checkDeadline()
			   └─（若 60s 无 write）close socket

*/

#include "httpConnection.h"
#include "LogicSystem.h"

httpConnection::httpConnection(boost::asio::io_context& ioc) :_socket(ioc)//socket没有拷贝构造,把本次通信给到一个httpconnection对象
{
}

void httpConnection::Start()
{
	auto self = shared_from_this();
	self->checkDeadline();//启动定时器

	//流程：从 socket 读取字节,存入 _buffer, 读取HTTP 报文填充 _request
	http::async_read(_socket, _buffer, _request, [self](beast::error_code ec, std::size_t bytes_transferred)//固定参数：错误和消耗的字节数 
		{
		try
		{
			if (ec)//错误就报错返回
			{
				std::cout << "http read error is" << ec.what() << std::endl;
				return;
			}

			boost::ignore_unused(bytes_transferred);//beast一般忽略处理消耗的字节数
			self->handleReq();//接受到http后开始处理请求，handleReq先解析请求，解析成功后再由writeResponse写回
		}
		catch (const std::exception& exp)
		{
			std::cout << "exception is" << exp.what() << std::endl;
		}
		});
}

void httpConnection::PreParseGetParam()
{
	//解析url，要么成功得到一个 url_view（只读的url），要么包含解析错误的返回值对象，
	auto result = boost::urls::parse_origin_form(_request.target());

	if (!result)//如果解析错误，就写回错误404
	{
		_response.result(http::status::bad_request);
		writeResponse();//写回
		return;
	}

	const auto& url = result.value();//得到真正的url
	_get_path = std::string(url.path());//获取path

	for (auto const& p : url.params())//获取query
	{
		_get_query.emplace(p.key, p.value);
	}
}

void::httpConnection::handleReq()
{
	_response.version(_request.version());//设置回复版本为客户端的http版本
	_response.keep_alive(false);//明确使用短链接：http为短链接，不用保证存活

	if (_request.method() == http::verb::get)//如果是get请求：_request.method()得到的是请求类型
	{
		PreParseGetParam();
		//连接是否成功的处理逻辑由 LogicSystem 类去处理
		bool success = LogicSystem::GetInstance()->handleGet(_get_path, shared_from_this());

		if (!success)//404错误的处理逻辑
		{
			_response.result(http::status::not_found);//设置_response状态码为404,
			_response.set(http::field::content_type, "text/plain");//响应头，相应头决定返回的格式
			beast::ostream(_response.body()) << "url not found\r\n";//body
			writeResponse();//写回  
			return; 
		}

		//成功路径就开始写回
		_response.result(http::status::ok);
		_response.set(http::field::server, "GateServer");
		writeResponse();
		return;
	}

	if (_request.method() == http::verb::post) 
	{
		bool success = LogicSystem::GetInstance()->handlePost(_request.target(), shared_from_this());
		if (!success) 
		{
			_response.result(http::status::not_found);
			_response.set(http::field::content_type, "text/plain");
			beast::ostream(_response.body()) << "url not found\r\n";
			writeResponse();
			return;
		}

		_response.result(http::status::ok);
		_response.set(http::field::server, "GateServer");
		writeResponse();
		return;
	}
}

void httpConnection::writeResponse()
{
	auto self = shared_from_this();
	_response.content_length(_response.body().size());//显式设置，没有 Content-Length，客户端可能阻塞

	//将 HTTP 响应序列化,写入 socket,完整写完后才回调。根据_response的状态码来返回信息
	http::async_write(_socket, _response, [self](beast::error_code ec, std::size_t bytes_transferred) 
		{
		self->_socket.shutdown(tcp::socket::shutdown_send, ec);//告诉对端我不再发数据了再关闭本端
		self->_deadline.cancel();//发送完就取消定时器，防止触发超时逻辑
		});
}

void httpConnection::checkDeadline()
{
	auto self = shared_from_this();
	_deadline.async_wait([self](beast::error_code ec) 
		{
		if (!ec)//长时间没有触发写回调，定时器没有正常取消，60s过后就会在这里强制关闭
		{
			self->_socket.close(ec);
		}
		});
}