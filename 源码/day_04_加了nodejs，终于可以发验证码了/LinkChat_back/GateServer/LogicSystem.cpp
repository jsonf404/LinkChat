#include "LogicSystem.h"
#include "httpConnection.h"
#include "VerifyGrpcClient.h"

void LogicSystem::RegGet(std::string url, httpHandler handler)
{
	_get_handlers[url] = handler;//把url绑定到一个可调用处理
}

void LogicSystem::RegPost(std::string url, httpHandler handler)
{
	_post_handlers[url] = handler;//把url绑定到一个可调用处理
}

LogicSystem::LogicSystem()
{
	/*在函数构造时先注册一个设定好的路由用于平时测试。
	此时调用RegGet，向map中插入了一个key：路由，value：被转换成function的lambda表达式
	之后根据路由找到value时，调用value就可以执行这个lambda表达式*/
	RegGet("/get_test", [](std::shared_ptr<httpConnection> connection) 
	{
		beast::ostream(connection->_response.body()) << "receive get_test req"<<std::endl;

		int i = 0;
		for (auto& elem : connection->_get_query)
		{
			i++;
			beast::ostream(connection->_response.body()) << "param" << i << " key is " << elem.first;
			beast::ostream(connection->_response.body()) << ", " << " value is " << elem.second << std::endl;
		}
		});

	//处理post请求验证码的请求，//http的url本身不是json格式，但是post请求的数据一般在body里面，body是json格式
	RegPost("/get_verifycode",[](std::shared_ptr<httpConnection> connection)
		{
			connection->_response.set(http::field::content_type, "application/json");//客户端会根据 Content-Type 决定如何解析。告诉客户端：返回内容是 JSON
			std::string body =boost::beast::buffers_to_string(connection->_request.body().data());//从post的body读取消息转换成string
			std::cout << "receive body: " << body << std::endl;//输出一下读到的消息

			Json::Value req;//存放客户端请求 JSON
			Json::Value resp;//存放服务端返回 JSON
			Json::Reader reader;// JsonCpp 解析器

			if (!reader.parse(body, req))//无法解析客户端发来的json
			{
				resp["error"] = ErrorCodes::Error_Json;//返回错误类型
				resp["msg"] = "invalid json";
				beast::ostream(connection->_response.body()) << resp.toStyledString();
				return true;
			}
			if (!req.isMember("email") || !req["email"].isString())//解析成功后检查传来的body是否有email
			{
				resp["error"] = ErrorCodes::Error_Json;
				resp["msg"] = "missing email";
				beast::ostream(connection->_response.body()) << resp.toStyledString();
				return true;
			}

			std::string email = req["email"].asString();//都正确了，得到email的内容
			std::cout << "email: " << email << std::endl;

			GetVarifyRsp grpc_rsp = VerifyGrpcClient::GetInstance()->GetVarifyCode(email);//同步调用 gRPC 服务获取验证码

			resp["error"] = ErrorCodes::Success;//返回json内容
			resp["email"] = email;
			beast::ostream(connection->_response.body()) << resp.toStyledString();//写回给客户端的纯字符串，不是json格式
			return true;
		});

}

bool LogicSystem::handleGet(std::string path, std::shared_ptr<httpConnection> connect)
{
	//检查路由是否注册
	auto it = _get_handlers.find(path);
	if (it == _get_handlers.end())
	{
		return false;
	}

	/*it是一个pair，第二个参数类型是httpHandler，上面在构造函数初始化时里面插入了一个pair，其第二个元素就是一个lambda表达式，
	即以参数connect调用了那个lambda表达式*/
	it->second(connect);
	return true;
}

bool LogicSystem::handlePost(std::string path, std::shared_ptr<httpConnection> connect)
{
	auto it = _post_handlers.find(path);
	if (it == _post_handlers.end())
	{
		return false;
	}

	it->second(connect);//不用返回了，返回类型都在lambda表达式里面了
	return true;
}
