#pragma once
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <memory>
#include<iostream>
#include "Singleton.h"
#include <unordered_map>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include <boost/url.hpp>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

enum ErrorCodes
{
	Success = 0,
    Error_Json = 1001,
	RPCFailed = 1002,
	VerifyExpired=1003,//验证码过期
	VerifyCodeError=1004,//验证码错误
	UserExist=1005,//用户存在
	PasswordError=1006,//密码错误
	EmailNotMatch=1007,//邮箱不匹配
	PasswordupdataError=1008,//更新密码失败
	RedisError=1009,//redis连接错误
};

class ConfigMgr;
extern ConfigMgr gCfgMgr;