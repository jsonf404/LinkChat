//#pragma once
//#include "const.h"
//#include "Singleton.h"
//#include "ConfigMgr.h"
//#include <grpcpp/grpcpp.h> 
//#include "message.grpc.pb.h"
//#include "message.pb.h"
//
//using grpc::Channel;
//using grpc::Status;
//using grpc::ClientContext;
//
//using message::GetChatServerReq;
//using message::GetChatServerRsp;
//using message::LoginRsp;
//using message::LoginReq;
//using message::StatusService;
//
///*
//作用：管理 gRPC 到 StatusServer 的连接池，
//负责创建、分配、回收 StatusService::Stub（即 gRPC 客户端对象），提高并发性能和资源复用。
//
//用途：让多个线程/请求可以高效地复用 gRPC 连接，避免每次都新建连接，提升服务端性能。
//*/
//class StatusConPool
//{
//public:
//	StatusConPool(std::size_t poolSize, std::string host, std::string port);
//
//	~StatusConPool();
//
//	//获取一个 gRPC 连接（StatusService::Stub），如果没有可用连接则等待
//	std::unique_ptr<StatusService::Stub> getConnection();
//
//	//归还一个 gRPC 连接（StatusService::Stub）到连接池
//	void returnConnection(std::unique_ptr<StatusService::Stub> context);
//
//	void Close();
//
//private:
//	std::atomic<bool> b_stop_;//标记连接池是否停止，停止后不再提供连接
//	std::size_t poolSize_; //连接池大小
//	std::string host_;
//	std::string port_;
//	std::queue<std::unique_ptr<StatusService::Stub>> connections_;//连接池中的 gRPC 连接队列
//	std::mutex mutex_;
//	std::condition_variable cond_;//条件变量，用于等待/通知连接可用
//};
//
//
///*
//作用：作为业务层的 gRPC 客户端单例，封装了与 StatusServer 的交互逻辑（如 GetChatServer、Login），
//对外提供接口，内部通过连接池管理 gRPC stub。
//
//用途：业务代码只需调用 StatusGrpcClient::GetInstance()->GetChatServer(...) 等方法，不关心底层连接细节。
//*/
//class StatusGrpcClient :public Singleton<StatusGrpcClient>
//{
//	friend class Singleton<StatusGrpcClient>;
//public:
//	~StatusGrpcClient() { }
//	GetChatServerRsp GetChatServer(int uid);
//	LoginRsp Login(int uid, std::string token);
//
//private:
//	StatusGrpcClient();
//	std::unique_ptr<StatusConPool> pool_;//连接池对象，负责管理 gRPC 连接
//};
//
//
//
