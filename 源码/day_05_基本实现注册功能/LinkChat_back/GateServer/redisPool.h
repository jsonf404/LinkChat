#pragma once

#include "Singleton.h"
#include "ConfigMgr.h"
#include <sw/redis++/redis++.h>
#include "const.h"

class RedisPool : public Singleton<RedisPool>
{
    friend class Singleton<RedisPool>;

public:
    // 获取连接
    std::shared_ptr<sw::redis::Redis> getConnection();

    // 归还连接
    void returnConnection(std::shared_ptr<sw::redis::Redis> conn);

    // 关闭池子（一般只在程序退出时）
    void Close();

    // 构造函数：构造即初始化池子
    RedisPool();
    ~RedisPool();

private:
    std::atomic<bool> b_stop_{ false };
    size_t poolSize_{ 0 };

    std::queue<std::shared_ptr<sw::redis::Redis>> connections_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

