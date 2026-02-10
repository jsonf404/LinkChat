#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. 创建堆栈控件（只 set 一次 centralWidget）,把页面都压入栈里面
    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    // 2. 创建页面（注意：现在它们只是“页面”，不是 dialog 语义）
    login_Widget = new LoginDialog(this);
    register_Widget = new RegisterDialog(this);

    // 3. 加入堆栈
    stack->addWidget(login_Widget);
    stack->addWidget(register_Widget);

    // 4. 默认显示登录页
    stack->setCurrentWidget(login_Widget);

    // 5. 处理各个页面发来的页面切换信号
    connect(login_Widget, &LoginDialog::switchRegister, this, &MainWindow::Slots_switchRegister);

    connect(register_Widget, &RegisterDialog::backToLogin, this, &MainWindow::Slots_backToLogin);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 登陆界面点击注册按钮后的槽函数事件
void MainWindow::Slots_switchRegister()
{
    stack->setCurrentWidget(register_Widget);
}

void MainWindow::Slots_backToLogin()
{
    stack->setCurrentWidget(login_Widget);
}
