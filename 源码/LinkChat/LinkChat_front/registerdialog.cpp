#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "global.h"
#include "httpmgr.h"
#include "QAction"
#include "QMessageBox"

RegisterDialog::RegisterDialog(QWidget *parent) : QDialog(parent), ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    ui->getverify_pushButton->setEnabled(false); // 先不能获取验证码，邮箱对了才能获取

    // 设置密码显示与否的逻辑
    ui->password_lineEdit->setEchoMode(QLineEdit::Password); // lineedit变成密码模式
    ui->confirmpassworld_lineEdit->setEchoMode(QLineEdit::Password);
    QAction *togglePwdAction = new QAction(this); // 新建一个action
    togglePwdAction->setCheckable(true);
    togglePwdAction->setIcon(QIcon(":/images/eye_close.png"));
    ui->password_lineEdit->addAction(togglePwdAction, QLineEdit::TrailingPosition); // 在输入行右侧嵌入一个action
    connect(togglePwdAction, &QAction::toggled, this, [=](bool checked) {
        if (checked)
        {
            ui->password_lineEdit->setEchoMode(QLineEdit::Normal);
            ui->confirmpassworld_lineEdit->setEchoMode(QLineEdit::Normal);
            togglePwdAction->setIcon(QIcon(":/images/eye_open.png"));
        }
        else
        {
            ui->password_lineEdit->setEchoMode(QLineEdit::Password);
            ui->confirmpassworld_lineEdit->setEchoMode(QLineEdit::Password);
            togglePwdAction->setIcon(QIcon(":/images/eye_close.png"));
        }
    });

    // 输入框内提示词
    ui->usr_lineEdit->setPlaceholderText("请输入用户名");
    ui->email_lineEdit->setPlaceholderText("请输入邮箱");
    ui->password_lineEdit->setPlaceholderText("请输入密码");
    ui->confirmpassworld_lineEdit->setPlaceholderText("请再次确认密码");
    ui->verify_lineEdit->setPlaceholderText("请输入验证码");

    // 一系列的检测输入信息是否合理的信号和槽连接，输入完就lambda检测是否符合要求。
    // connect最后一个参数要传入的是函数地址，用lambda比较好，不推荐用类名::函数名的写法。直接写函数名会报错
    connect(ui->usr_lineEdit, &QLineEdit::editingFinished, this, [this]() { checkUserValid(); });
    connect(ui->email_lineEdit, &QLineEdit::editingFinished, this, [this]() { checkEmailValid(); });
    connect(ui->password_lineEdit, &QLineEdit::editingFinished, this, [this]() { checkPassValid(); });
    connect(ui->confirmpassworld_lineEdit, &QLineEdit::editingFinished, this, [this]() { checkConfirmpassValid(); });
    connect(ui->verify_lineEdit, &QLineEdit::editingFinished, this, [this]() { checkVerifyValid(); });

    // 在发送http请求且接收到回复，会根据回复的模块发送对应的信号给各模块。表示这个回包是给回给哪个模块的
    // 现在这个回包是回给注册模块的，交给注册模块去处理详细信息
    connect(httpmgr::GetInstance().get(), &httpmgr::sig_reg_mod_finish, this, &RegisterDialog::Slots_reg_mod_finish);

    // 处理各种回包逻辑的函数
    inithttpHandlers();

    // 点击取消按钮发出返回登陆界面的信号
    connect(ui->cancel_pushButton, &QPushButton::clicked, this, &RegisterDialog::backToLogin);
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

// 对输入信息的一系列要求
bool RegisterDialog::checkUserValid()
{
    if (ui->usr_lineEdit->text() == "")
    {
        showMessage("用户名不能为空", "red");
        return false;
    }
    return true;
}

bool RegisterDialog::checkPassValid()
{
    auto pass = ui->password_lineEdit->text();

    // ^[a-zA-Z0-9!@#$%^&*]{6,15}$ 密码长度至少6，可以是字母、数字和特定的特殊字符
    static const QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*]{6,15}$");
    if (!regExp.match(pass).hasMatch())
    {
        showMessage("密码应为6-15位的无非法字符组合", "red");
        ui->password_lineEdit->clear();
        return false;
    }
    return true;
}

bool RegisterDialog::checkConfirmpassValid()
{
    auto pass = ui->password_lineEdit->text();
    auto con_pass = ui->confirmpassworld_lineEdit->text();
    if (pass != con_pass)
    {
        showMessage("密码冲突,请确认密码", "red");
        ui->confirmpassworld_lineEdit->clear();
        return false;
    }
    return true;
}

bool RegisterDialog::checkEmailValid()
{
    // 验证邮箱的地址正则表达式
    auto email = ui->email_lineEdit->text();
    // 邮箱地址的正则表达式
    static const QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    if (!regex.match(email).hasMatch())
    {
        showMessage("邮箱地址错误", "red");
        ui->email_lineEdit->clear();
        ui->getverify_pushButton->setEnabled(false);
        return false;
    }
    ui->getverify_pushButton->setEnabled(true);
    return true;
}

bool RegisterDialog::checkVerifyValid()
{
    auto verify = ui->verify_lineEdit->text();

    static const QRegularExpression re(R"(^\d{4}$)");

    if (!re.match(verify).hasMatch())
    {
        showMessage("验证码应为四位数字", "red");
        ui->verify_lineEdit->clear();
        return false;
    }
    return true;
}

// 更改提示文字和颜色的函数
void RegisterDialog::showMessage(const QString &msg, const QString &color)
{
    ui->error_tips->setStyleSheet("color: " + color + "; font-weight: bold;");
    ui->error_tips->setText(msg);
}

// 注册成功后返回到登陆界面的倒计时
void RegisterDialog::backToLogin_CountDown(int seconds)
{
    auto box = new QMessageBox(QMessageBox::Information, tr("注册成功"), tr("注册成功，%1 秒后返回登录").arg(seconds), QMessageBox::NoButton, this);
    box->show();

    auto timer = new QTimer(this);
    timer->setInterval(1000);

    connect(timer, &QTimer::timeout, this, [=]() mutable {
        seconds--;
        if (seconds <= 0)
        {
            timer->stop();
            box->close();
            emit backToLogin(); // 5s后返回登陆界面
        }
        else
        {
            box->setText(tr("注册成功，%1 秒后返回登录").arg(seconds));
        }
    });
    timer->start();
}

// 获取验证码的请求
void RegisterDialog::on_getverify_pushButton_clicked()
{
    auto email = ui->email_lineEdit->text();
    showMessage("获取验证码请求发出！", "green");

    QJsonObject json_obj; // 发送http post请求获取验证码
    json_obj["email"] = email;
    httpmgr::GetInstance()->postHttpreq(QUrl(gate_url_prefix + "/get_verifycode"), json_obj, ReqId::GetVerifyCode, Module::Register);
    qDebug() << "获取验证码请求发出，url为:" << QUrl(gate_url_prefix + "/get_verifycode").toString();
}

// 注册的按钮，post注册请求
void RegisterDialog::on_confirm_pushButton_clicked()
{
    bool valid = checkUserValid();
    if (!valid)
    {
        return;
    }

    valid = checkEmailValid();
    if (!valid)
    {
        return;
    }

    valid = checkPassValid();
    if (!valid)
    {
        return;
    }

    valid = checkConfirmpassValid();
    if (!valid)
    {
        return;
    }

    valid = checkVerifyValid();
    if (!valid)
    {
        return;
    }
    ui->error_tips->clear();

    // json向网关服务器发送注册模块的申请
    QJsonObject json_obj;
    json_obj["user"] = ui->usr_lineEdit->text();
    json_obj["email"] = ui->email_lineEdit->text();
    json_obj["passwd"] = sha256(ui->password_lineEdit->text()); // 加密密码
    // json_obj["confirm"] = sha256(ui->confirmpassworld_lineEdit->text());
    json_obj["varifycode"] = ui->verify_lineEdit->text();
    httpmgr::GetInstance()->postHttpreq(QUrl(gate_url_prefix + "/user_register"), json_obj, ReqId::RegisterUser, Module::Register);
}

// 处理注册模块信号回包信息的槽函数。这部分处理只关心回包是否能解析可用
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

// 为不同 HTTP 请求的 可解析能用的 回包注册对应的处理函数。// 根据ReqId不同处理注册不同的处理函数
// 这部分是业务逻辑部分，解决：回包可以解析能用，但业务逻辑成不成功的问题
void RegisterDialog::inithttpHandlers()
{
    // 此时捕获的是 GetVerifyCode，验证码部分。即QT发了验证码请求给网关服务器后接收到的返回结果处理逻辑。
    _handlers.insert(ReqId::GetVerifyCode, [this](const QJsonObject &jsonObj) {
        int error = jsonObj["error"].toInt();
        if (static_cast<ErrorCode>(error) != ErrorCode::Success)
        {
            showMessage("验证码获取失败！", "red");
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
            showMessage("注册失败！", "red");
            return;
        }

        showMessage("注册成功！", "green");
        qDebug() << "用户注册成功，邮箱是：" << jsonObj["email"].toString();

        this->setEnabled(false);  // 跳转过程不可操作
        backToLogin_CountDown(5); // 五秒后返回登陆界面
    });
}
