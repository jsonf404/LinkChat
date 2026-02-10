#include "timerbtn.h"
#include <QMouseEvent>
// 先初始化父类构造函数，再赋值给倒计时60s，验证码60s发送一次
TimerBtn::TimerBtn(QWidget *parent) : QPushButton(parent), _counter(15)
{
    _timer = new QTimer(this);
    // 绑定一个信号，timeout信号每被触发一次就调用一次回调函数
    connect(_timer, &QTimer::timeout, this, [this]() {
        _counter--;
        if (_counter <= 0) // 倒计时结束
        {
            _timer->stop();
            _counter = 15;
            this->setText("重新获取");
            this->setEnabled(true);
            return;
        }
        this->setText("(" + QString::number(_counter) + "s)后重发");
    });
}

TimerBtn::~TimerBtn()
{
    _timer->stop(); // 优雅回收
}

// 重写鼠标离开的事件，鼠标离开按钮就会触发回调
void TimerBtn::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        // 在这里处理鼠标左键释放事件
        this->setEnabled(false);
        _timer->start(1000); // 一秒触发一次回调，即1s发送一个timeout信号
        emit clicked();      // 抛出一个按下的信号
    }
    // 调用基类的mouseReleaseEvent以确保正常的事件处理（如点击效果）
    QPushButton::mouseReleaseEvent(e);
}
