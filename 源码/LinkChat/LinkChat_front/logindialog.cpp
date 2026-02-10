#include "logindialog.h"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent), ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    // 绑定登陆界面的注册按钮，事件是点击，点击后触发登陆界面自己定义的一个信号，此信号被绑定在了mainwindow的一个槽函数里面，最终目的是触发那个槽函数
    // 这么麻烦就是为了实现信号和槽的解耦，实现各窗口的解耦
    connect(ui->register_pushButton, &QPushButton::clicked, this, &LoginDialog::switchRegister);

    ui->lineEdit->setPlaceholderText("请输入用户名");
    ui->lineEdit_2->setPlaceholderText("请输入密码");
}

LoginDialog::~LoginDialog()
{
    delete ui;
}
