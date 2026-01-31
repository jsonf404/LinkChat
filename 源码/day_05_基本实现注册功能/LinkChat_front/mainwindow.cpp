#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 登陆界面和主界面连接
    login_dialog = new LoginDialog();
    setCentralWidget(login_dialog); // 登陆界面放到主界面中心

    // 登陆界面和注册界面的连接，登陆界面的 switchRegister 信号触发后，执行MainWindow::Slots_switchRegister这个槽函数
    connect(login_dialog, &LoginDialog::switchRegister, this, &MainWindow::Slots_switchRegister);
    register_dialog = new RegisterDialog();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 登陆界面点击注册按钮后的槽函数事件
void MainWindow::Slots_switchRegister()
{
    setCentralWidget(register_dialog); // 设置注册界面到主界面
    // login_dialog->hide();//隐藏登陆界面
}
