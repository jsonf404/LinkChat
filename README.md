# C++全栈即时通讯项目

> **项目描述：**一个全栈即时通讯系统，采用C/S架构。客户端基于Qt5实现，服务端采用微服务分布式架构



## 技术栈

**技 术 栈：**C++、Boost.Asio、gRPC、Redis、MySQL、Qt5、llama.cpp、GTest、Benchmark，多线程/异步编程，分布式系统设计



## 架构设计

<img src="img/1.png" alt="1" style="zoom:50%;" />

- GateServer为网关服务，主要应对客户端的连接和注册请求，因为服务器是是分布式，所以GateServer收到用户连接请求后会查询状态服务选择一个负载较小的Server地址给客户端，客户端拿着这个地址直接和Server通信建立长连接。

- 当用户注册时会发送给GateServer, GateServer调用VarifyServer验证注册的合理性并发送验证码给客户端，客户端拿着这个验证码去GateServer注册即可。

- StatusServer， ServerA， ServerB都可以直接访问Redis和Mysql服务。
