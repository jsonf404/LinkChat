/*
GateServer网关类只要验证码结果，本类就是专门处理验证码的中转站：

GateServer网关类，向本类VerifyGrpcClient发送邮箱以获取验证码
即LogicSystem里的post请求在解析到qt客户端发来的email后，将那个email给到本类，本类再发给nodejs调用官方的api接口

官方api接口会：
1. 发送验证码到具体设备
2. 同时把验证码回传给本类，本类再给到网关类

只有 VerifyGrpcClient <-> VerifyServer(nodejs文件) 这一段使用了 gRPC + Protobuf 通信
*/

/*
这个类其实也是一个rpcpool连接池管理类，在这个类的构造函数里面初始化连接池，每次只调用一个连接，用完还回去
*/

#pragma once

#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "const.h"
#include "Singleton.h"
using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;

//rpc连接池
class RPCconPool
{
public:
    RPCconPool(size_t poolsize, std::string host, std::string port);

    //析构函数，在对象消亡前，确保没有线程再使用内部资源
    ~RPCconPool() 
    {
        std::lock_guard<std::mutex> lock(mutex_);//析构线程必须等当前持锁的操作完成
        Close();//先广播，再清理。保证wait线程唤醒并安全退出
        while (!_connections.empty())
        {
            _connections.pop();//清空连接池
        }
    }

    void Close();

    std::unique_ptr<VarifyService::Stub> getConnection();

    void returnConnection(std::unique_ptr<VarifyService::Stub> context);

private:
    std::atomic<bool> b_stop_;
    size_t _poolsize;
    std::string _host;
    std::string _port;
    std::queue<std::unique_ptr<VarifyService::Stub>> _connections;//唯一持有stub，Stub 就是“本地对象和远程服务”的桥梁
    std::condition_variable cond_;//C++11 引入的线程同步原语，防止线程忙等空转消耗cpu，让线程直接休眠，条件满足再唤醒，不消耗cpu
    std::mutex mutex_;//互斥，控制队列线程安全
};

class VerifyGrpcClient :public Singleton<VerifyGrpcClient>//单例，全局只需要一个gRPC客户端
{
    friend class Singleton<VerifyGrpcClient>;//强制单例模式

public:
    GetVarifyRsp GetVarifyCode(std::string email);

private:
    VerifyGrpcClient();
    std::unique_ptr<RPCconPool> _pool;
};

