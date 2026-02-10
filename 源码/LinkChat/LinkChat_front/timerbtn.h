#ifndef TIMERBTN_H
#define TIMERBTN_H
#include <QPushButton>
#include <QTimer>

/*
 继承于pushbutton的子类，用于给基本按钮升级
 给获取验证码按钮升级的一个类，升级后获取验证码的按钮就有了这个倒计时的功能
*/
class TimerBtn : public QPushButton
{
public:
    TimerBtn(QWidget *parent = nullptr);
    ~TimerBtn();

    // 重写mouseReleaseEvent
    virtual void mouseReleaseEvent(QMouseEvent *e) override;

private:
    QTimer *_timer;
    int _counter;
};
#endif // TIMERBTN_H
