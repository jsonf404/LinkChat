#include "ConfigMgr.h"
#include "CServer.h"

namespace net = boost::asio;

//#include <sw/redis++/redis++.h>
//int test()
//{
//    try {
//        // 连接Redis服务器
//        sw::redis::Redis redis("tcp://127.0.0.1:6380");
//
//        // 如果有密码
//        redis.auth("123456");
//
//        // 测试连接
//        redis.ping();
//        std::cout << "Redis连接成功！" << std::endl;
//
//    }
//    catch (const sw::redis::Error& e) {
//        std::cerr << "Redis错误: " << e.what() << std::endl;
//    }
//
//    return 0;
//}

int main()
{
    try
    {
        // 1. 构造 ConfigMgr（构造函数中已加载 config.ini）
        ConfigMgr gCfgMgr;

        // 2. 从配置读取 GateServer.Port
        const SectionInfo& gateSection = gCfgMgr.getSection("GateServer");

        std::string gate_port_str = gateSection.get("Port");
        if (gate_port_str.empty())
        {
            throw std::runtime_error("GateServer.Port not found in config.ini");
        }

        int port_cfg = std::stoi(gate_port_str);//把端口转换成int
        if (port_cfg <= 0 || port_cfg > 65535)
        {
            throw std::runtime_error("invalid GateServer.Port value");
        }

        unsigned short port = static_cast<unsigned short>(port_cfg);
        // 3. io_context
        net::io_context ioc{ 1 };

        // 4. 信号处理
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const boost::system::error_code& error, int)
            {
                if (error)
                {
                    return;
                }
                ioc.stop();
            });

        // 5. 启动服务器
        std::make_shared<CServer>(ioc, port)->Start();

        // 6. 事件循环
        ioc.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}