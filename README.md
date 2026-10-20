# LinkChat

LinkChat 是一个面向学习和二次开发的桌面即时通信项目，采用 C/S 架构。客户端使用 Qt Widgets 构建；C++ 服务端拆分为网关、状态、聊天等服务；独立的 Node.js 验证服务负责验证码派发。服务间使用 gRPC，客户端通过 HTTP 完成注册、登录和密码重置等流程，并通过 TCP 长连接进行聊天通信。

> 当前仓库包含 Qt `.pro`、Visual Studio `.sln/.vcxproj` 和 CMake 三种构建入口。CMake 配置覆盖 Qt 客户端与 C++ 服务端；Node.js 验证服务仍通过 npm 单独运行。项目依赖较多，首次构建需要先安装并配置第三方库和运行服务。

## 目录结构

```text
LinkChat/
├── CMakeLists.txt              # CMake 总入口
├── CMakePresets.json           # default / client-only / servers-only 预设
├── client/
│   └── linkchat/               # Qt 客户端源码、界面、资源和 Qt .pro 工程
├── server/
│   ├── GateServer/             # HTTP 网关、注册/登录入口
│   ├── StatusServer/           # 状态查询与聊天节点选择
│   ├── ChatServer/             # TCP 聊天服务实例
│   ├── ChatServer2/            # 第二份聊天服务工程，可作为另一实例
│   └── VarifyServer/           # Node.js + gRPC 邮箱验证码服务
├── sqldata/
│   └── linkchat.sql            # 数据库表结构及演示数据
├── 开发文档/                    # 按开发阶段整理的说明文档
│   └── img/                    # 文档使用的本地图片
└── img/                        # 项目图片资源
```

## 功能概览

### Qt 客户端

- 注册、登录、密码重置界面及邮箱验证码交互。
- 好友搜索、好友申请、申请处理和联系人列表。
- 聊天列表、聊天页面、文本气泡和图片消息展示。
- 使用 Qt Network 封装 HTTP 请求；使用 TCP 管理类与聊天服务通信。
- 使用 Qt Designer `.ui` 文件构建界面，以 Qt Resource System 打包图标、头像和样式表。

### C++ 服务端

- **GateServer**：基于 Boost.Asio/Beast 提供 HTTP 接口，协调验证、数据库访问和状态服务。
- **StatusServer**：提供 gRPC 状态查询，并依据在线负载信息选择聊天节点。
- **ChatServer**：基于 Boost.Asio 提供 TCP 长连接，并通过 gRPC 与其他节点协作。
- **ChatServer2**：独立的第二聊天服务工程，可用于多节点部署或对照开发。
- **共享数据服务**：MySQL 用于用户、好友和申请等数据；Redis 用于验证码、在线状态、节点信息等运行数据。

### 验证服务

`server/VarifyServer` 使用 Node.js、`@grpc/grpc-js`、Redis 和 Nodemailer 提供验证码服务。邮件 SMTP 配置需要使用自己的邮箱及授权码，不能直接使用示例占位配置。

## 架构与请求流程

```text
                         ┌──────────────────┐
                         │   VarifyServer   │
                         │ Node.js / gRPC   │
                         └────────▲─────────┘
                                  │ gRPC
┌──────────────┐  HTTP   ┌────────┴─────────┐     gRPC      ┌──────────────┐
│ Qt Client    ├────────►│   GateServer     ├──────────────►│ StatusServer │
└──────┬───────┘         └───────┬──────────┘               └──────┬───────┘
       │ TCP                     │                                  │ gRPC
       │                         │                                  ▼
       └─────────────────────────┼──────────────────────────►┌──────────────┐
                                 │                           │ ChatServer A│
                         ┌───────▼──────┐                    │ / ChatServer2│
                         │ MySQL/Redis  │                    └──────────────┘
                         └──────────────┘
```

典型登录流程是客户端通过 HTTP 请求 GateServer，GateServer 校验账号并向 StatusServer 查询可用聊天节点，然后将连接信息返回客户端；客户端再直接与指定 ChatServer 建立 TCP 连接。注册流程会调用 VarifyServer 发送验证码，校验通过后再创建用户记录。

## 技术栈

| 层次 | 技术 |
| --- | --- |
| 客户端 | C++、Qt Widgets、Qt Network、Qt Designer、Qt Resource System |
| HTTP / TCP | Boost.Asio、Boost.Beast |
| RPC | gRPC、Protocol Buffers |
| 数据存储 | MySQL、MySQL Connector/C++、Redis、hiredis |
| JSON | JsonCpp |
| 验证服务 | Node.js、npm、`@grpc/grpc-js`、Nodemailer、ioredis |
| 构建 | CMake 3.20+、Visual Studio 工程、qmake `.pro` 工程 |

## 构建环境

### 必需工具

- Windows 10/11 x64（仓库中现有 Visual Studio 工程以 Windows 为目标）。
- CMake 3.20 或更高版本。
- Visual Studio 2019/2022 C++ 工具链，或其他兼容 C++14 的编译器。
- Qt 5 或 Qt 6，包含 Widgets、Network、UIC、MOC 和 RCC 工具。
- Boost（至少 1.70；项目代码使用 Asio、Beast、UUID、PropertyTree 和 Filesystem）。
- Protobuf 和 gRPC C++ 开发包。
- JsonCpp、MySQL Connector/C++、hiredis。
- Node.js 和 npm（仅运行邮箱验证码服务时需要）。
- MySQL、Redis 服务端（运行完整业务链路时需要）。

依赖库必须与目标架构、编译器 ABI 和运行时匹配。Windows 下建议统一使用 x64 和同一 MSVC 工具集；不要混用 x86/x64、不同 MSVC 版本或不兼容的 Debug/Release 库。

## 使用 CMake 构建

顶层配置提供三个预设：

| 预设 | 用途 |
| --- | --- |
| `client-only` | 仅配置 Qt 客户端 |
| `servers-only` | 仅配置四个 C++ 服务 |
| `default` | 同时配置客户端与 C++ 服务 |

预设使用 Ninja 生成器，因此需要先安装 Ninja 并确保 `ninja` 可从命令行调用。通过 `CMAKE_PREFIX_PATH` 提供 Qt、gRPC、Protobuf 和 JsonCpp 等依赖的 CMake package 路径；MySQL Connector/C++ 和 hiredis 使用项目定义的专用根路径变量。

### 仅构建客户端

```powershell
cmake --preset client-only `
  -DCMAKE_PREFIX_PATH="D:/Qt/6.6.0/msvc2019_64"

cmake --build --preset client-only
```

请将示例 Qt 路径替换为本机安装位置。Qt 5 用户也可将路径指向相应的 Qt 5 MSVC kit。

### 仅配置服务端

```powershell
cmake --preset servers-only `
  -DCMAKE_PREFIX_PATH="D:/deps;D:/Qt/6.6.0/msvc2019_64" `
  -DLINKCHAT_MYSQL_ROOT="D:/deps/mysql-connector" `
  -DLINKCHAT_HIREDIS_ROOT="D:/deps/hiredis"

cmake --build --preset servers-only
```

`D:/deps` 应包含相应依赖的 CMake package 配置（例如 gRPC、Protobuf、JsonCpp）；Boost 还需要确保头文件和相应库可被 CMake/编译器找到。若使用 vcpkg，可通过 `CMAKE_TOOLCHAIN_FILE` 指向其工具链文件，并按需设置 triplet：

```powershell
cmake --preset servers-only `
  -DCMAKE_TOOLCHAIN_FILE="D:/vcpkg/scripts/buildsystems/vcpkg.cmake" `
  -DVCPKG_TARGET_TRIPLET="x64-windows" `
  -DLINKCHAT_MYSQL_ROOT="D:/deps/mysql-connector" `
  -DLINKCHAT_HIREDIS_ROOT="D:/deps/hiredis"
```

MySQL Connector/C++ 的安装布局和库名称因版本而异；如果 CMake 未找到 `jdbc/mysql_driver.h` 或 `mysqlcppconn` 库，请检查 `LINKCHAT_MYSQL_ROOT` 下的 include/lib 目录。hiredis 同理检查头文件和库文件是否位于其安装目录中。

### 使用 Visual Studio 生成器

也可以绕过 Ninja 预设，直接选择 Visual Studio 生成器：

```powershell
cmake -S . -B build/vs2022 `
  -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_PREFIX_PATH="D:/deps;D:/Qt/6.6.0/msvc2019_64" `
  -DLINKCHAT_MYSQL_ROOT="D:/deps/mysql-connector" `
  -DLINKCHAT_HIREDIS_ROOT="D:/deps/hiredis"

cmake --build build/vs2022 --config Release
```

### Protobuf / gRPC 源码

各 C++ 服务目录已经提交 `message.pb.cc/.h` 和 `message.grpc.pb.cc/.h`。默认构建使用这些已生成文件。修改 `.proto` 后，可以配置 `-DLINKCHAT_REGENERATE_PROTO=ON` 重新生成；生成时要求当前 Protobuf 与 gRPC 工具版本和项目依赖兼容，并且 `protoc`、`grpc_cpp_plugin` 均可由对应 CMake package 提供。

## 使用 Qt Creator / qmake

客户端仍保留 qmake 工程 [client/linkchat/LinkChat.pro](client/linkchat/LinkChat.pro)：

1. 安装与编译器匹配的 Qt 5/Qt 6 kit。
2. 在 Qt Creator 中打开 `client/linkchat/LinkChat.pro`。
3. 选择 x64 kit，运行 qmake 并构建。
4. 将客户端 `config.ini` 放在程序输出目录，客户端启动时会从可执行文件目录读取它。

qmake 工程的部署步骤还会尝试复制 `config.ini` 和 `static` 目录；请按实际部署内容准备这些资源。

## 运行准备

仓库没有提供完整的运行时配置文件。C++ 服务通过当前工作目录下的 `config.ini` 读取配置，客户端读取可执行文件目录下的 `config.ini`；验证服务通过工作目录下的 `config.json` 读取 Redis 和 SMTP 邮箱配置。启动前请根据代码读取的 section/key 建立自己的配置文件，并使用本机服务地址和账号。不要将真实邮箱授权码、数据库密码或 Redis 密码提交到版本库。

配置字段可从以下源码确认：

- C++ 服务连接项：各服务目录的 `MysqlDao.cpp`、`RedisMgr.cpp`、`ConfigMgr.cpp`。
- 节点地址：各服务的 `StatusGrpcClient.cpp`、`VerifyGrpcClient.cpp`、`ChatGrpcClient.cpp`。
- 邮件和 Redis 配置：`server/VarifyServer/config.js`、`server/VarifyServer/email.js`。
- 客户端 GateServer 地址：`client/linkchat/main.cpp` 中读取的 `GateServer/host`、`GateServer/port`。

### 数据库初始化

`sqldata/linkchat.sql` 包含建表语句和演示记录。请先新建专用开发数据库，再确认目标数据库为空或数据已备份后导入。该 SQL 会执行 `DROP TABLE IF EXISTS` 并重建部分表，**不要直接在有重要数据的数据库上执行**。服务端配置中的 MySQL schema 必须与导入目标数据库一致。

### Node.js 验证服务

```powershell
cd server/VarifyServer
npm ci
node server.js
```

服务监听地址在源码中绑定为 `0.0.0.0:50051`。启动前创建符合 `config.js` 读取结构的 `config.json`，至少提供 `email.user`、`email.pass`、`mysql.host`、`mysql.port`、`redis.host`、`redis.port` 和 `redis.passwd`。当前验证逻辑实际依赖邮件与 Redis 配置；请为每个环境填写自己的值。邮件发送需要有效 SMTP 邮箱及应用授权码。

### 服务启动顺序

确保 MySQL 和 Redis 已运行并且配置连接可用后，建议按依赖顺序启动：

1. `VarifyServer`：邮箱验证码 gRPC 服务。
2. `StatusServer`：节点状态与选择服务。
3. 至少一个 `ChatServer` 实例（需要多节点时再启动 `ChatServer2`）。
4. `GateServer`：客户端 HTTP 入口。
5. Qt 客户端。

各 C++ 服务从**当前工作目录**读取 `config.ini`，因此启动时工作目录必须包含该服务对应配置。每个服务的监听地址、端口和下游地址均由配置及源码决定；启动前检查端口没有冲突，并确保 GateServer、StatusServer、ChatServer 和 VarifyServer 使用一致的地址配置。

## 常见问题排查

### CMake 找不到 Qt

检查 `CMAKE_PREFIX_PATH` 是否指向 Qt 对应编译器 kit 的根目录，确认该 kit 安装了 Widgets、Network 和开发工具，并检查位数与当前生成器一致。

### CMake 找不到 Boost

确保 Boost 头文件可见；如果 Boost 安装不在系统默认路径，设置 `BOOST_ROOT`/`Boost_ROOT` 或将 Boost 安装前缀加入 `CMAKE_PREFIX_PATH`。部分 Boost 组件可能还需要对应编译库。

### 找不到 gRPC / Protobuf / JsonCpp

将它们的 CMake package 安装前缀加入 `CMAKE_PREFIX_PATH`，并检查包是否和当前 MSVC、架构及配置匹配。已有生成的 `.pb.cc` 文件并不意味着可以省略 Protobuf/gRPC 的头文件和链接库。

### MySQL Connector/C++ 或 hiredis 链接失败

检查 `LINKCHAT_MYSQL_ROOT`、`LINKCHAT_HIREDIS_ROOT`、include/lib 目录、库文件名及 Debug/Release 版本。运行时还需确保依赖 DLL 位于可执行文件目录或 `PATH` 中。

### 服务启动时找不到配置或无法连接

确认进程工作目录下存在 `config.ini`（Node 服务则为 `config.json`），各配置 key 拼写与代码中的读取项一致，并确认数据库/Redis/内部 RPC 端口可达。错误配置常表现为配置读取异常、连接池初始化失败或 RPC 请求超时。

### 客户端 HTTP 请求失败

核对客户端 `config.ini` 中 GateServer 的 host/port，确认 GateServer 正在监听该地址，并确认 Windows 防火墙或本机代理没有拦截请求。

## 安全与数据注意事项

- 当前项目面向本地开发与学习，不应直接作为公网生产服务部署。
- gRPC 示例使用不安全凭据的配置时，不具备 TLS 加密能力；生产部署应增加 TLS、身份认证、访问控制和网络隔离。
- 不要使用真实用户数据或真实密码导入演示 SQL；演示表中可能含有明文测试凭据。
- 不要把 `config.ini`、`config.json`、私钥、SMTP 授权码、数据库口令或 Redis 口令提交到公共仓库。
- 修改 SQL 前先备份数据；导入脚本会删除并重建表。
- 邮箱验证码、账号校验、日志、异常处理和并发安全仍需结合实际部署要求进行安全审计。

## 开发文档

`开发文档/` 按阶段记录客户端、HTTP/TCP 服务、gRPC、Redis、MySQL、好友关系与聊天通信等实现过程。文档图片已存放在 `开发文档/img/`，Markdown 使用相对路径引用，离线浏览时也可正常显示。

## 已知边界

- 运行配置模板未随仓库提供；需根据本机环境自行创建。
- Qt、Boost、gRPC、MySQL Connector/C++、hiredis 等依赖需单独安装。
- `ChatServer2` 是单独工程目录，构建成功不等于已完成多节点运行配置；端口、节点名称和 StatusServer 中的节点清单需匹配。
- CMake 配置解决工程组织与依赖发现，不会自动安装 Redis/MySQL 服务，也不会自动创建数据库或准备邮箱账号。
- `server/VarifyServer` 是 Node.js 服务，不由 CMake 构建。

## 许可证与来源

使用、修改或再分发前，请确认原始代码、文档、图片和第三方依赖各自适用的许可证与授权范围。第三方库仍受其各自许可证约束。
