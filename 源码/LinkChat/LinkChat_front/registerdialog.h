#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "global.h"

namespace Ui
{
    class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

    void showMessage(const QString &msg, const QString &color); // 更改提示文字和颜色的函数

private slots:
    void on_getverify_pushButton_clicked(); // 获取验证码按钮

    void Slots_reg_mod_finish(ReqId id, QString res, ErrorCode err); // 处理注册模块信号的槽函数

    void on_confirm_pushButton_clicked(); // 确认注册按钮

private:
    Ui::RegisterDialog *ui;

    // 回包的json解析成功就添加到_handlers对象里面，由inithttpHandlers()具体处理
    QMap<ReqId, std::function<void(const QJsonObject &)>> _handlers;

    void inithttpHandlers(); // 为每一个回包添加不同的处理逻辑

    bool checkUserValid(); // 对注册信息的一系列要求
    bool checkEmailValid();
    bool checkPassValid();
    bool checkConfirmpassValid();
    bool checkVerifyValid();

    void backToLogin_CountDown(int seconds); // 注册成功后返回登陆界面的倒计时显示

signals:
    void backToLogin(); // 返回登陆界面的信号
};

#endif // REGISTERDIALOG_H
