#include "AsioiocontextPool.h"
#include <iostream>
using namespace std;

//在初始化池的大小的时候就初始化了io_context和Work的数目,有默认参数，两个
AsioiocontextPool::AsioiocontextPool(std::size_t size) :_ioc(size),_works(size), _nextioc(0) 
{
    for (std::size_t i = 0; i < size; ++i) 
    {
        //保证work和ioc生命周期绑定，保证等会run()的时候，没有任务ioc也不会直接返回
        _works[i] = std::unique_ptr<Work>(new Work(_ioc[i].get_executor()));
    }

    //遍历多个ioservice，创建多个线程，每个线程内部启动ioservice
    for (std::size_t i = 0; i < _ioc.size(); ++i) 
    {
        _threads.emplace_back([this, i]() {
            _ioc[i].run();
            });
    }
}

AsioiocontextPool::~AsioiocontextPool() 
{
    Stop();//先清空池子，再析构
    std::cout << "AsioIOServicePool destruct" << endl;
}

boost::asio::io_context& AsioiocontextPool::Getioc() 
{
    auto& service = _ioc[_nextioc++];
    if (_nextioc == _ioc.size())
    {
        _nextioc= 0;
    }
    return service;
}

void AsioiocontextPool::Stop()
{
    _works.clear();//先把work和ioc的绑定解除

    for (auto& ioc : _ioc)//手动stop每个ioc
    {
        ioc.stop();
    }

    for (auto& t : _threads)//等待所有线程安全结束
    {
        if (t.joinable())
            t.join();
    }
}
