#include "httpmgr.h"

httpmgr::~httpmgr()
{
}

httpmgr::httpmgr()
{
    connect(this, &httpmgr::sig_http_finish, this, &httpmgr::Slots_sig_http_finish); // 发送http请求的信号和处理这个信号的槽函数连接
}

// 统一发送 HTTP POST 请求，并在请求完成后，通过信号把结果“按请求类型 + 模块”分发出去。
void httpmgr::postHttpreq(QUrl url, QJsonObject json, ReqId req_id, Module mod)
{
    QByteArray data = QJsonDocument(json).toJson(); // 先将对象转成json字节流,即变成http能发送的字节流
    QNetworkRequest request(url);                   // 构造请求对象

    // 设置http头
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");                  // 请求体是json格式
    request.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(data.length())); // 明确字段长度

    // 伪闭包，确保异步回调期间httpmgr对象不会被提前析构
    auto self = shared_from_this();

    QNetworkReply *reply = _manager.post(request, data); // 正式的发请求

    connect(reply, &QNetworkReply::finished, [self, reply, req_id, mod]() { // 处理发送完请求后的处理逻辑
        if (reply->error() != QNetworkReply::NoError)
        {
            qDebug() << reply->errorString();

            // 只发信号，不弹UI，emit只是一个强调发送了信号的宏，没有实际作用。发出一个信号：我发送成功，但接收失败
            emit self->sig_http_finish(req_id, "", ErrorCode::NetworkErr, mod);
            reply->deleteLater();
            return;
        }

        // 无错误
        QString res = reply->readAll();                                   // 读取服务器返回的json文本，暂时不解析
        emit self->sig_http_finish(req_id, res, ErrorCode::Success, mod); // 发出一个信号：我发送成功，且接收成功
        reply->deleteLater();
        return;
    });
}

void httpmgr::Slots_sig_http_finish(ReqId id, QString res, ErrorCode err, Module mod)
{
    if (mod == Module::Register) // 处理发送http请求后返回的信号，如果信号对应注册模块，就发送注册模块信号
    {
        emit sig_reg_mod_finish(id, res, err);
    }
}
