#include "registerdialog.h"
#include "global.h"
#include "ui_registerdialog.h"

RegisterDialog::RegisterDialog(QWidget *parent) : QDialog(parent), ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);

    ui->passworld_lineEdit->setEchoMode(QLineEdit::Password);
    ui->confirmpassworld_lineEdit->setEchoMode(QLineEdit::Password);

    connect(ui->checkBox, &QCheckBox::toggled, this,
            [=](bool checked)
            {
                ui->passworld_lineEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
                ui->confirmpassworld_lineEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
            });
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

// 更改提示文字和颜色的函数
void RegisterDialog::showMessage(const QString &msg, const QString &color)
{
    ui->error_tips->setStyleSheet("color: " + color + "; font-weight: bold;");
    ui->error_tips->setText(msg);
}

// 槽函数 验证邮箱是否正确，正确就发验证码
void RegisterDialog::on_getvarify_pushButton_clicked()
{
    QString email = ui->email_lineEdit->text();

    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)"); // 邮箱地址的正则表达式
    bool               match = regex.match(email).hasMatch();         // 执行正则表达式匹配

    if (match)
    {
        // 邮箱地址正确
        showMessage("发送成功！", "green");
        // 发送http请求获取验证码
    }
    else
    {
        // 提示邮箱不正确
        showMessage("邮箱地址错误！", "red");
    }
}
