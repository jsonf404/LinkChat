//#include "StatusGrpcClient.h"
//
////状态服务器连接池实现，在构造函数中创建指定数量的 gRPC 连接（StatusService::Stub）并放入队列中，
//StatusConPool::StatusConPool(size_t poolSize, std::string host, std::string port)
//	: poolSize_(poolSize), host_(host), port_(port), b_stop_(false) {
//	for (size_t i = 0; i < poolSize_; ++i) 
//	{
//
//		std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port,
//			grpc::InsecureChannelCredentials());
//
//		connections_.push(StatusService::NewStub(channel));
//	}
//}
//
//StatusConPool::~StatusConPool()
//{
//	std::lock_guard<std::mutex> lock(mutex_);
//	Close();
//	while (!connections_.empty()) {
//		connections_.pop();
//	}
//}
//
////获取一个 gRPC 连接（StatusService::Stub），如果没有可用连接则等待
//std::unique_ptr<StatusService::Stub> StatusConPool::getConnection() 
//{
//	std::unique_lock<std::mutex> lock(mutex_);
//	cond_.wait(lock, [this] {
//		if (b_stop_) {
//			return true;
//		}
//		return !connections_.empty();
//		});
//
//	if (b_stop_) { return  nullptr; }
//	auto context = std::move(connections_.front());
//	connections_.pop();
//	return context;
//}
//
////归还一个 gRPC 连接（StatusService::Stub）到连接池
//void StatusConPool::returnConnection(std::unique_ptr<StatusService::Stub> context) 
//{
//	std::lock_guard<std::mutex> lock(mutex_);
//	if (b_stop_) {
//		return;
//	}
//	connections_.push(std::move(context));
//	cond_.notify_one();
//}
//
//void StatusConPool::Close() 
//{
//	b_stop_ = true;
//	cond_.notify_all();
//}
//
////StatusGrpcClient类定义
////调用 gRPC 的 GetChatServer 方法，获取聊天服务器信息。
//GetChatServerRsp StatusGrpcClient::GetChatServer(int uid)
//{
//	ClientContext context;
//	GetChatServerRsp reply;
//	GetChatServerReq request;
//	request.set_uid(uid);
//	auto stub = pool_->getConnection();
//	Status status = stub->GetChatServer(&context, request, &reply);
//
//	Defer defer([&stub, this]() {
//		pool_->returnConnection(std::move(stub));
//		});
//
//	if (status.ok()) 
//	{
//		return reply;
//	}
//	else 
//	{
//		reply.set_error(ErrorCodes::RPCFailed);
//		return reply;
//	}
//}
//
////调用 gRPC 的 Login 方法，进行用户登录验证。
//LoginRsp StatusGrpcClient::Login(int uid, std::string token)
//{
//	ClientContext context;
//	LoginRsp reply;
//	LoginReq request;
//	request.set_uid(uid);
//	request.set_token(token);
//
//	auto stub = pool_->getConnection();
//	Status status = stub->Login(&context, request, &reply);
//
//	Defer defer([&stub, this]() {
//		pool_->returnConnection(std::move(stub));
//		});
//
//	if (status.ok()) {
//		return reply;
//	}
//	else {
//		reply.set_error(ErrorCodes::RPCFailed);
//		return reply;
//	}
//}
//
////StatusGrpcClient类定义
//StatusGrpcClient::StatusGrpcClient()
//{
//	//从ConfigMgr中获取配置，初始化连接池。
//	auto& cfgMgr = ConfigMgr::instance();
//	const auto& sec = cfgMgr.getSection("StatusServer");
//	std::string host = sec.get("Host");
//	std::string port = sec.get("Port");
//	pool_.reset(new StatusConPool(5, host, port));
//}