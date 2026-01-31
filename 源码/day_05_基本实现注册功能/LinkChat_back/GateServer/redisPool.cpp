#include "RedisPool.h"

RedisPool::RedisPool()
{
    // 1. 从配置中心读取 Redis 配置
    auto& cfgMgr = ConfigMgr::instance();
    const auto& sec = cfgMgr.getSection("Redis");

    std::string host = sec.get("Host");
    int port = std::stoi(sec.get("Port"));
    std::string pwd = sec.get("PassWord");
    poolSize_ = 5;

    // 2. redis++ 连接配置
    sw::redis::ConnectionOptions opts;
    opts.host = host;
    opts.port = port;
    opts.password = pwd;
    opts.socket_timeout = std::chrono::milliseconds(2000);

    // 3. 初始化连接池
    for (size_t i = 0; i < poolSize_; ++i)
    {
        try {
            auto redis = std::make_shared<sw::redis::Redis>(opts);
            connections_.push(redis);
        }
        catch (const sw::redis::Error& e) {
            std::cerr << "[RedisPool] 连接失败: " << e.what() << std::endl;
        }
    }

    if (connections_.empty()) 
    {
        std::cerr << "[RedisPool] 警告：没有成功创建任何 Redis 连接" << std::endl;
    }
}

RedisPool::~RedisPool()
{
    Close();

    std::lock_guard<std::mutex> lock(mutex_);
    while (!connections_.empty()) {
        connections_.pop(); // shared_ptr 自动析构
    }
}

std::shared_ptr<sw::redis::Redis> RedisPool::getConnection()
{
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] {
        return b_stop_ || !connections_.empty();
        });

    if (b_stop_) {
        return nullptr;
    }

    auto conn = connections_.front();
    connections_.pop();
    return conn;
}

void RedisPool::returnConnection(std::shared_ptr<sw::redis::Redis> conn)
{
    if (!conn) return;

    std::lock_guard<std::mutex> lock(mutex_);
    if (b_stop_) return;

    connections_.push(conn);
    cond_.notify_one();
}

void RedisPool::Close()
{
    b_stop_ = true;
    cond_.notify_all();
}
