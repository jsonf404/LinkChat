#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "global.h"
#include "httpmgr.h"

RegisterDialog::RegisterDialog(QWidget *parent) : QDialog(parent), ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);

    // 控制密码显示与否
    ui->password_lineEdit->setEchoMode(QLineEdit::Password);
    ui->confirmpassworld_lineEdit->setEchoMode(QLineEdit::Password);
    connect(ui->checkBox, &QCheckBox::toggled, this, [=](bool checked) {
        ui->password_lineEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
        ui->confirmpassworld_lineEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    });

    // 在发送http请求且接收到回复，且发送对应信号后，如果这个信号对应的是注册模块，也会发送一个对应注册模块的信号
    // 此时就把最终发送的这个注册模块信号绑定到其对应处理注册模块的槽函数上
    connect(httpmgr::GetInstance().get(), &httpmgr::sig_reg_mod_finish, this, &RegisterDialog::Slots_reg_mod_finish);

    inithttpHandlers(); // 处理各种回包逻辑的函数
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

// 槽函数 验证邮箱是否正确，正确就post发验证码的请求
void RegisterDialog::on_getverify_pushButton_clicked()
{
    QString email = ui->email_lineEdit->text();

    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)"); // 邮箱地址的正则表达式
    bool match = regex.match(email).hasMatch();                       // 执行正则表达式匹配

    if (match)
    {
        // 邮箱地址正确
        showMessage("发送成功！", "green");

        // 发送http post请求获取验证码
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

// 注册界面的个人注册的按钮，post注册请求
void RegisterDialog::on_confirm_pushButton_clicked()
{
    if (ui->usr_lineEdit->text() == "")
    {
        showMessage("用户名不能为空！", "red");
        return;
    }

    if (ui->email_lineEdit->text() == "")
    {
        showMessage("邮箱不能为空！", "red");
        return;
    }

    if (ui->password_lineEdit->text() == "")
    {
        showMessage("密码不能为空！", "red");
        return;
    }

    if (ui->confirmpassworld_lineEdit->text() != ui->password_lineEdit->text())
    {
        showMessage("密码冲突，请确认密码！", "red");
        return;
    }

    if (ui->verify_lineEdit->text() == "")
    {
        showMessage("验证码不能为空！", "red");
        return;
    }

    // json向网关服务器发送注册模块的申请
    QJsonObject json_obj;
    json_obj["user"] = ui->usr_lineEdit->text();
    json_obj["email"] = ui->email_lineEdit->text();
    json_obj["passwd"] = ui->password_lineEdit->text();
    json_obj["confirm"] = ui->confirmpassworld_lineEdit->text();
    json_obj["varifycode"] = ui->verify_lineEdit->text();
    httpmgr::GetInstance()->postHttpreq(QUrl(gate_url_prefix + "/user_register"), json_obj, ReqId::RegisterUser, Module::Register);
}

// 处理注册模块信号的槽函数
void RegisterDialog::Slots_reg_mod_finish(ReqId id, QString res, ErrorCode err)
{
    if (err == ErrorCode::NetworkErr) // 注册模块信号有两个:发送成功，接收成功/失败，先处理这两个信息
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
        showMessage("解析成功转换失败！", "red");
    }

    jsonDoc.object(); // 都没问题就转换成数据结构对象
    // 服务器返回的json对象，在这里根据id的值执行回包的处理。但具体逻辑在下面的inithttpHandlers()函数里面，是lambda表达式
    _handlers[id](jsonDoc.object());
    return;
}

// 为不同 HTTP 请求的返回结果（回包）注册对应的处理函数。// 根据ReqId不同处理注册不同的处理函数
void RegisterDialog::inithttpHandlers()
{
    // 此时捕获的是 GetVerifyCode，验证码部分。即QT发了验证码请求给网关服务器后接收到的返回结果处理逻辑。
    _handlers.insert(ReqId::GetVerifyCode, [this](const QJsonObject &jsonObj) {
        int error = jsonObj["error"].toInt();
        if (static_cast<ErrorCode>(error) != ErrorCode::Success) // 这个返回格式是服务器指定的，暂时这样写
        {
            showMessage("参数错误！", "red");
            return;
        }

        showMessage("验证码已经发送到邮箱，注意查收！", "green");
    });

    // 代码后续继续扩展，捕获其他枚举值....

    // 处理用户注册的回包逻辑
    _handlers.insert(ReqId::RegisterUser, [this](QJsonObject jsonObj) {
        int error = jsonObj["error"].toInt();
        if (error != static_cast<int>(ErrorCode::Success)) // 强枚举类型，要显示转换
        {
            showMessage("参数错误！", "red");
            return;
        }

        showMessage("注册成功！", "red");
    });
}
