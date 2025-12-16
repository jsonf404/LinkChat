#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>

namespace Ui
{
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

    void showMessage(const QString &msg, const QString &color); // 更改提示文字和颜色的函数

  private slots:
    void on_getvarify_pushButton_clicked();

  private:
    Ui::RegisterDialog *ui;
};

#endif // REGISTERDIALOG_H
