/*
AsioiocontextPool 是一个“io_context 线程池 + 负载分发器 + 生命周期管理器”的单例组件。
多个线程，每个线程有一个ioc
*/
#include <vector>
#include <boost/asio.hpp>
#include "Singleton.h"

class AsioiocontextPool :public Singleton<AsioiocontextPool>//奇异递归，保证单例类,全局只有一个线程池
{
    friend Singleton<AsioiocontextPool>;//构造函数是私有的，只有单例模板可以调用他，防止外部new
public:
    using io_context = boost::asio::io_context;
    //io_context没有任务会立即退出，work防止 io_context.run() 在“暂时没任务”时退出
    using Work = boost::asio::executor_work_guard<io_context::executor_type>;
    using WorkPtr = std::unique_ptr<Work>;

    ~AsioiocontextPool();
    
    boost::asio::io_context& Getioc();//继续得到上下文
    void Stop();//停止服务池

private:
    AsioiocontextPool(std::size_t size = 2/*std::thread::hardware_concurrency()*/);//构造函数，线程多少
    std::vector<io_context> _ioc;//存上下文的容器
    std::vector<WorkPtr> _works;//每个ioc有一个work
    std::vector<std::thread> _threads;//线程集合
    std::size_t _nextioc;//实现轮询分配，一个请求一个ioc 
};