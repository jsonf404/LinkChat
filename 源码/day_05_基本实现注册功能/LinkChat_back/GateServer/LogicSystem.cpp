#include "LogicSystem.h"
#include "httpConnection.h"
#include "VerifyGrpcClient.h"
#include "redisPool.h"

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
	/*
	在函数构造时先注册一个设定好的路由用于平时测试。
	此时调用RegGet，向map中插入了一个key：路由，value：被转换成function的lambda表达式
	之后根据路由找到value时，调用value就可以执行这个lambda表达式
	*/
	//测试代码
	/*RegGet("/get_test", [](std::shared_ptr<httpConnection> connection)
	{
		beast::ostream(connection->_response.body()) << "receive get_test req"<<std::endl;

		int i = 0;
		for (auto& elem : connection->_get_query)
		{
			i++;
			beast::ostream(connection->_response.body()) << "param" << i << " key is " << elem.first;
			beast::ostream(connection->_response.body()) << ", " << " value is " << elem.second << std::endl;
		}
		});*/

		//处理客户端post请求验证码的请求，//http的url本身不是json格式，但是post请求的数据一般在body里面，body是json格式
	RegPost("/get_verifycode", [](std::shared_ptr<httpConnection> connection)
		{
			std::string body = boost::beast::buffers_to_string(connection->_request.body().data());//从post的body读取消息转换成string
			std::cout << "接收到的验证码请求体是: " << body << std::endl;//输出一下读到的消息

			//明确告诉客户端：响应体里的字节是 UTF-8 编码的 JSON 数据，从而让客户端和框架按 JSON 自动、正确地解析它。避免无提示解析出错
			connection->_response.set(http::field::content_type, "application/json; charset=utf-8");

			Json::Value req;//存放客户端请求的 JSON
			Json::Value resp;//存放服务端返回的 JSON
			Json::Reader reader;// JsonCpp 解析器

			if (!reader.parse(body, req))//无法解析客户端发来的json
			{
				std::cout << "解析验证码请求(josn)失败!" << std::endl;
				resp["error"] = ErrorCodes::Error_Json;//返回错误类型
				resp["message"] = "解析验证码请求(josn)失败!";
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
			std::cout << "解析成功得到的email是: " << email << std::endl;

			//同步调用 gRPC 服务获取验证码。此时就创建了VerifyGrpcClient这个单例，自动初始化了rpc连接池且只初始化一次
			GetVarifyRsp grpc_rsp = VerifyGrpcClient::GetInstance()->GetVarifyCode(email);

			resp["error"] = ErrorCodes::Success;//返回json内容
			resp["email"] = email;

			//将 JSON 对象格式 序列化为 JSON 字符串文本，写入 HTTP body。QT端再根据json格式解析出对应的json对象格式
			beast::ostream(connection->_response.body()) << resp.toStyledString();
			return true;
		});

	// 处理客户端 post 用户注册请求
	RegPost("/user_register", [](std::shared_ptr<httpConnection> connection)
		{
			auto body = boost::beast::buffers_to_string(connection->_request.body().data());
			std::cout << "接收到的注册请求体为：" << body << std::endl;

			connection->_response.set(http::field::content_type,"application/json; charset=utf-8");

			Json::Value resp;
			Json::Value req;
			Json::Reader reader;

			if (!reader.parse(body, req))
			{
				std::cout << "解析注册请求(json)失败"<<std::endl;
				resp["error"] = ErrorCodes::Error_Json;
				resp["message"] = "解析注册请求(json)失败";
				beast::ostream(connection->_response.body()) << resp.toStyledString();
				return true;
			}

			auto pool = RedisPool::GetInstance();

			// ================== 1. 校验验证码 ==================
			std::string varify_code;

			auto redis = pool->getConnection();
			if (!redis) 
			{
				std::cout << "redis连接失败" << std::endl;
				resp["error"] = ErrorCodes::RedisError;
				resp["message"] = "redis unavailable";
				beast::ostream(connection->_response.body()) << resp.toStyledString();
				return true;
			}

			try
			{
				auto val = redis->get("code_" + req["email"].asString());//nodejs代码往redis里面插入的验证码有“code_” 前缀
				if (!val)
				{
					std::cout << "验证码过期"<< std::endl;
					pool->returnConnection(redis);
					resp["error"] = ErrorCodes::VerifyExpired;
					resp["message"] = "验证码过期";
					beast::ostream(connection->_response.body()) << resp.toStyledString();
					return true;
				}

				varify_code = *val;
				pool->returnConnection(redis);
			}
			catch (const sw::redis::Error& e)
			{
				std::cout << "捕获到异常" << std::endl;
				pool->returnConnection(redis);
				resp["error"] = ErrorCodes::RedisError;
				resp["message"] = "redis error";
				beast::ostream(connection->_response.body()) << resp.toStyledString();
				return true;
			}


			if (varify_code != req["varifycode"].asString())
			{
				std::cout << "验证码错误" << std::endl;
				resp["error"] = ErrorCodes::VerifyCodeError;
				resp["message"] = "验证码错误";
				beast::ostream(connection->_response.body()) << resp.toStyledString();
				return true;
			}

			// ================== 3. 后续逻辑,返回给客户端 ==================
			resp["error"] = 0;
			resp["email"] = req["email"];
			resp["user"] = req["user"].asString();
			resp["passwd"] = req["passwd"].asString();
			resp["confirm"] = req["confirm"].asString();
			resp["varifycode"] = req["varifycode"].asString();

			beast::ostream(connection->_response.body()) << resp.toStyledString();
			std::cout << "注册成功，已将注册信息回传给客户端" << std::endl;
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
