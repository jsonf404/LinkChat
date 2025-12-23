#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "global.h"
#include "httpmgr.h"

RegisterDialog::RegisterDialog(QWidget *parent) : QDialog(parent), ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);

    // 控制密码显示与否
    ui->passworld_lineEdit->setEchoMode(QLineEdit::Password);
    ui->confirmpassworld_lineEdit->setEchoMode(QLineEdit::Password);
    connect(ui->checkBox, &QCheckBox::toggled, this, [=](bool checked) {
        ui->passworld_lineEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
        ui->confirmpassworld_lineEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    });

    // 在发送http请求且接收到回复，且发送对应信号后，如果这个信号对应的是注册模块，也会发送一个对应注册模块的信号
    // 此时就把最终发送的这个注册模块信号绑定到其对应处理注册模块的槽函数上
    connect(httpmgr::GetInstance().get(), &httpmgr::sig_reg_mod_finish, this, &RegisterDialog::Slots_reg_mod_finish);

    inithttpHandlers();
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
    bool match = regex.match(email).hasMatch();                       // 执行正则表达式匹配

    if (match)
    {
        // 邮箱地址正确
        showMessage("发送成功！", "green");

        // 发送http请求获取验证码
        QJsonObject json_obj;
        json_obj["email"] = email;
        httpmgr::GetInstance()->postHttpreq(QUrl(gate_url_prefix + "/get_verifycode"), json_obj, ReqId::GetVerifyCode, Module::Register);
        qDebug() << "Final Post URL:" << QUrl(gate_url_prefix + "/get_verifycode").toString();
    }
    else
    {
        // 提示邮箱不正确
        showMessage("邮箱地址错误！", "red");
    }
}

// 处理注册模块信号的槽函数
void RegisterDialog::Slots_reg_mod_finish(ReqId id, QString res, ErrorCode err)
{
    if (err != ErrorCode::Success) // 注册模块信号有两个:发送成功，接收成功/失败，先处理这两个信息
    {
        showMessage("网络请求错误！", "red");
        return;
    }

    // 接受成功就解析json字符串，接受字符串res转ByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8()); // QJsonDocument反序列化，把字节流01转换成json文件
    if (jsonDoc.isNull())                                          // 解析失败为空
    {
        showMessage("json解析失败！", "red");
        return;
    }

    if (!jsonDoc.isObject()) // 能解析，但是不能转换成数据结构对象
    {
        showMessage("网络请求错误！", "red");
    }

    jsonDoc.object();                // 都没问题就转换成数据结构对象
    _handlers[id](jsonDoc.object()); // 服务器返回的json对象，id对应的值。id是在发信号时就确定是哪个枚举值了
    return;
}

// 为不同 HTTP 请求的返回结果（回包）注册对应的处理函数。// 根据ReqId不同处理注册不同的处理函数
void RegisterDialog::inithttpHandlers()
{
    // 此时捕获的是 GetVerifyCode，验证码部分
    _handlers.insert(ReqId::GetVerifyCode, [this](const QJsonObject &jsonObj) {
        int error = jsonObj["error"].toInt();
        if (static_cast<ErrorCode>(error) != ErrorCode::Success) // 这个返回格式是服务器指定的，暂时这样写
        {
            showMessage("参数错误！", "red");
            return;
        }

        auto email = jsonObj["email"].toString(); // 这个返回格式是服务器指定的，暂时这样写
        showMessage("验证码已经发送到邮箱，注意查收！", "green");
        qDebug() << "邮箱是：" << email;
    });

    // 代码后续继续扩展，捕获其他枚举值
}
