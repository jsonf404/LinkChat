#include "VerifyGrpcClient.h"
#include "ConfigMgr.h"

VerifyGrpcClient::VerifyGrpcClient()//构造函数初始化
{ 
    auto& cfgMgr = ConfigMgr::instance();
    const SectionInfo& sec = cfgMgr.getSection("VerifyServer");

    std::string host = sec.get("Host");
    std::string port = sec.get("Port");

    _pool.reset(new RPCconPool(5, host, port));//初始化连接池，在VerifyGrpcClient类调用单例方法时自动构造，初始化连接池
}

//跨进程调用获取验证码的服务，grpc。在logicsystem中被使用到
GetVarifyRsp VerifyGrpcClient::GetVarifyCode(std::string email)
{
    ClientContext context;//一次grpc的上下文
    GetVarifyRsp reply;//nodejs返回的验证码,这里是grpc
    GetVarifyReq request;//发送给nodejs
    request.set_email(email);

    //注意这个GetVarifyCode和函数名不是一个东西，这个GetVarifyCode是导入的包里面的东西，专门用来跨进程调用的
    Status status = _pool->getConnection()->GetVarifyCode(&context, request, &reply);//真正的跨进程调用nodejs，这里用到了grpc

    if (status.ok())
    {
        _pool->returnConnection(std::move(_pool->getConnection()));
        return reply;
    }
    else
    {
        reply.set_error(ErrorCodes::RPCFailed);
        return reply;
    }
}

//线程池
RPCconPool::RPCconPool(size_t poolsize, std::string host, std::string port)
    :_poolsize(poolsize), _host(host), _port(port) ,b_stop_(false)//连接池一开始是“可用状态”,所有等待线程默认可以正常工作
{
    for (size_t i = 0; i < _poolsize; i++)
    {
        //指定验证码服务地址，gRPC通信方式
        //每个线程一个信使
        std::shared_ptr<Channel> channel = grpc::CreateChannel(host+":"+port, grpc::InsecureChannelCredentials());
        _connections.push(VarifyService::NewStub(channel));
    }
}

//告诉其他挂起的线程，线程池要关闭了
void RPCconPool::Close()
{
    b_stop_ = true;//stop的状态切换，从现在开始，不允许再获取连接 / 继续工作
    cond_.notify_all();//唤醒其他在wait的线程，让他们知道已经stop了，池已关闭，安全退出线程逻辑
}

std::unique_ptr<VarifyService::Stub> RPCconPool::getConnection()
{
    std::unique_lock<std::mutex> lock(mutex_);

    //wait逻辑只返回bool值，表示我还要不要继续等待
    cond_.wait(lock, [this]() {
        if (b_stop_)
        {
            return true;//池关闭了，返回true只是提醒起来做善后工作
        }
        return !_connections.empty();//池没关闭，池空了就返回false继续等待，没空就返回true唤醒继续下面的逻辑
        });

    if (b_stop_) {
        return nullptr;//二次检查，连接池已关闭，不再分配资源
    }

    //池没关闭就给连接
    auto context=std::move(_connections.front());
    _connections.pop();
    return context;
}

void RPCconPool::returnConnection(std::unique_ptr<VarifyService::Stub> context)//归还连接
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (b_stop_)
    {
        return;
    }
    _connections.push(std::move(context));
    cond_.notify_one();//提醒下一个连接
}