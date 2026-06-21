#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QWidget>

namespace Ui { class LoginPage; }

class LoginPage : public QWidget
{
    Q_OBJECT

public:
    explicit LoginPage(QWidget *parent = nullptr);
    ~LoginPage();

signals:
    void loginSuccess();

private slots:
    void onLoginClicked();

private:
    Ui::LoginPage *ui;
};

#endif