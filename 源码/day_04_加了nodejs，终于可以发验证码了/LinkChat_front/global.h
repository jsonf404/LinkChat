#ifndef GLOBAL_H
#define GLOBAL_H

#include "QRegularExpression"
#include "QStyle"
#include <QWidget>
#include <iostream>
#include <memory>
#include <mutex>
#include <QNetworkReply>
#include <QJsonObject>

// 将一些事件可能选择与代码模块解耦，发生了一件事，我的枚举里面有所有事的状态枚举，不用在主代码里面疯狂ifelse
// 请求 ID：用于客户端与服务端通信协议
enum class ReqId : int
{
    GetVerifyCode = 1001, // 获取验证码
    RegisterUser = 1002,  // 注册用户
};

// 模块 ID：用于区分业务模块
enum class Module : int
{
    Register = 0, // 注册模块
};

// 错误码：统一的返回状态
enum class ErrorCode : int
{
    Success = 0,    // 成功
    JsonError = 1,  // JSON 解析错误
    NetworkErr = 2, // 网络错误
};

extern QString gate_url_prefix; // 网关服务器的url前缀（path之前的内容）
#endif                          // GLOBAL_H
