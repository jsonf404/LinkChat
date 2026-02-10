#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "logindialog.h"    //登陆界面
#include "registerdialog.h" //注册界面
#include <QMainWindow>
#include "QStackedWidget"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void Slots_switchRegister(); // 登陆界面点击注册的槽函数
    void Slots_backToLogin();    // 其他界面返回登陆界面

private:
    Ui::MainWindow *ui;
    QStackedWidget *stack;
    LoginDialog *login_Widget;       // 登陆界面实例
    RegisterDialog *register_Widget; // 注册界面实例
};
#endif // MAINWINDOW_H
