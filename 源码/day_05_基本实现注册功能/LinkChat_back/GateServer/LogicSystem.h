/*
是 HTTP 服务器中的“业务调度中心 / 路由系统”，负责把 URL 映射到具体的业务处理函数。
“某个请求来了 → 应该执行哪段业务逻辑”
*/
#pragma once
#include "const.h"

class httpConnection;
typedef std::function<void(std::shared_ptr<httpConnection>)> httpHandler;//减少头文件依赖、加快编译速度
/*
httpHandler = 一个“可调用对象”
			  接收：shared_ptr<httpConnection>，使用智能指针保证这条连接存活
			  返回：void

即一次 HTTP 请求，对应一次“业务处理函数”
*/

class LogicSystem :public Singleton<LogicSystem>//单例
{
	friend class Singleton<LogicSystem>;//允许 Singleton 访问私有构造函数

public:
	~LogicSystem() { }
	bool handleGet(std::string,std::shared_ptr<httpConnection>);//连接是否成功，就是httpConnection里面的 success
	bool handlePost(std::string, std::shared_ptr<httpConnection>);

	void RegGet(std::string, httpHandler handler);//负责建立URL → handler 映射，把 URL 和处理函数绑定起来
	void RegPost(std::string, httpHandler handler); 

private:
	LogicSystem();//私有构造函数，禁止外部创建，只能通过 GetInstance() 访问
	std::unordered_map<std::string, httpHandler> _post_handlers;//大量查找比map更快
	std::unordered_map<std::string, httpHandler> _get_handlers;
};
