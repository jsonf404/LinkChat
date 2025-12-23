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
    void on_getvarify_pushButton_clicked();
    void Slots_reg_mod_finish(ReqId id, QString res, ErrorCode err); // 处理注册模块信号的槽函数

private:
    Ui::RegisterDialog *ui;
    void inithttpHandlers(); // 初始化每一个http请求
    QMap<ReqId, std::function<void(const QJsonObject &)>> _handlers;
};

#endif // REGISTERDIALOG_H
