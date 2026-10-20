# LinkChat

> **项目描述：**一个全栈即时通讯系统，采用C/S架构。客户端基于Qt5实现，服务端采用微服务分布式架构。

**● 客户端：**界面仿主流IM产品交互并使用QSS优化，使用QListWidget实现好友列表，利用GridLayout和QPainter封装气泡聊天框组件等；基于QTNetwork模块封装HTTP与TCP通信模块。支持添加好友，即时通信，聊天记录展示等功能。

**● 服务端：**采用微服务分布式架构：拆分GateServer网关、ChatServer聊天（多实例）、StatusServer状态、VerifyServer验证四类服务，各服务间基于gRPC通信；GateServer对外提供HTTP登录/注册接口，结合StatusServer查询低负载节点实现ChatServer负载均衡；ChatServer基于Boost.Asio实现TCP长连接的异步收发与断线重连，采用多线程io_context池提升并发能力；基于MySQL完成数据持久化，并封装MySQL、Redis、gRPC三类连接池，支撑高并发访问与多服务协作。

**● AI聊天机器人：**接入本地部署的llama.cpp+Qwen大模型推理服务，通过 llama.cpp内置HTTP Server暴露的推理接口，在ChatServer中识别机器人会话并转发对话请求；基于Redis存储最近几轮对话作为上下文，实现具备基础记忆能力的智能聊天功能。内置好友，用户可在客户端直接对话。

● **测试与质量保障：**基于GTest/QTest/Benchmark对核心模块进行单元测试与性能基准测试，结合OpenCppCoverage分析并提升代码覆盖率。 

**技 术 栈：**C++、Boost.Asio、gRPC、Redis、MySQL、Qt5、llama.cpp、GTest、Benchmark，多线程/异步编程，分布式系统设计

**成** **果：**单节点压测支持8000+并发连接，长时间运行通信状态稳定；核心模块单元测试覆盖率达到85%

## CMake 构建

项目提供 CMake 构建文件，要求 CMake 3.20 或更高版本，C++14 编译器，以及以下依赖：Qt 5/6（客户端）、Boost、Protobuf、gRPC、JsonCpp、MySQL Connector/C++ 和 hiredis。

使用 Qt、gRPC 等依赖的 CMake 安装前缀设置 `CMAKE_PREFIX_PATH`，并通过 `LINKCHAT_MYSQL_ROOT` 与 `LINKCHAT_HIREDIS_ROOT` 指定 MySQL Connector/C++ 和 hiredis 的安装目录。例如：

```powershell
cmake --preset servers-only -DCMAKE_PREFIX_PATH="D:/cppsoft/grpc;D:/Qt/6.6.0/msvc2019_64" -DLINKCHAT_MYSQL_ROOT="D:/cppsoft/mysql-connector" -DLINKCHAT_HIREDIS_ROOT="D:/cppsoft/hiredis"
cmake --build --preset servers-only
```

客户端和服务端也可以分别使用 `client-only` 与默认 `default` 预设。仓库内已提交的 protobuf/gRPC C++ 文件默认直接参与构建；需要重新生成时可增加 `-DLINKCHAT_REGENERATE_PROTO=ON`。
