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
    void on_getverify_pushButton_clicked();
    void Slots_reg_mod_finish(ReqId id, QString res, ErrorCode err); // 处理注册模块信号的槽函数

    void on_confirm_pushButton_clicked();

private:
    Ui::RegisterDialog *ui;
    void inithttpHandlers();                                         // 为每一个回包添加不同的处理逻辑
    QMap<ReqId, std::function<void(const QJsonObject &)>> _handlers; // 回包的json解析成功就添加到_handlers对象里面，由inithttpHandlers()具体处理
};

#endif // REGISTERDIALOG_H
