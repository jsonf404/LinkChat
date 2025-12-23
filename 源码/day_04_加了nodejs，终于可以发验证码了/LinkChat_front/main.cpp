#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include "global.h"
#include <QDir>
#include <qsettings.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/images/logo.png"));

    QString fileName = "config.ini";                           // 生成 config.ini 的绝对路径（平台无关）
    QString app_path = QCoreApplication::applicationDirPath(); // 获取程序运行路径
    QString config_path = QDir::toNativeSeparators(app_path + QDir::separator() + fileName);
    QSettings settings(config_path, QSettings::IniFormat); // 以 INI 格式加载配置文件
    QString gate_host = settings.value("GateServer/host").toString();
    QString gate_port = settings.value("GateServer/port").toString(); // 读取网关服务器的配置项，即config文件里面的内容，只用在config里面改ip和端口就行，随时换
    gate_url_prefix = "http://" + gate_host + ":" + gate_port; // 拼装网关访问前缀

    MainWindow w;
    w.show();
    return a.exec();
}
