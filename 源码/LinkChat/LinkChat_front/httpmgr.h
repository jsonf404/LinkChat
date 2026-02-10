#ifndef HTTPMGR_H
#define HTTPMGR_H
#include "singleton.h"
#include <QString>
#include <QUrl>
#include <QObject>
#include <QNetworkAccessManager> //网络模块
#include <QJsonObject>           //序列化
#include <QJsonDocument>         //反序列化

// CRTP 奇异递归模板，是一种模板继承模式：派生类把“自己”作为模板参数传给基类。
// CRTP 是静态多态，虚函数是动态多态
class httpmgr : public QObject, public Singleton<httpmgr>, public std::enable_shared_from_this<httpmgr>
{
    Q_OBJECT
public:
    ~httpmgr(); // 基类模板在析构时要析构静态成员，静态成员的析构由智能指针管理，智能指针的析构类型是T，T就是派生类的类型，因此派生类的析构要是public

    void postHttpreq(QUrl url, QJsonObject json, ReqId req_id, Module mod); // 发送post请求，参数是路由，json，请求id，注册模块

private:
    friend class Singleton<httpmgr>; // 基类声明为派生类的友元函数，使基类可以访问子类构造函数
    httpmgr();

    QNetworkAccessManager _manager;

private slots:
    void Slots_sig_http_finish(ReqId id, QString res, ErrorCode err, Module mod); // 处理接收到：发送成功，接收成功/失败两种信号的槽函数

signals:
    void sig_http_finish(ReqId id, QString res, ErrorCode err, Module mod); // 发送http给服务器，返回：发送成功，接收成功/失败两种信号
    void sig_reg_mod_finish(ReqId id, QString res, ErrorCode err);          // 注册模块结束的信号
};

#endif // HTTPMGR_H
