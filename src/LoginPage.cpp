#include "LoginPage.h"
#include "ui_LoginPage.h"
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QDebug>

LoginPage::LoginPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::LoginPage)
{
    ui->setupUi(this);
    qDebug() << "LoginPage constructor";

    QPushButton *loginBtn = findChild<QPushButton*>("pushButtonSignIn");
    if (!loginBtn) {
        loginBtn = findChild<QPushButton*>("pushButtonLogin");
    }
    if (!loginBtn) {
        loginBtn = findChild<QPushButton*>("btnLogin");
    }
    if (!loginBtn) {
        loginBtn = findChild<QPushButton*>("loginButton");
    }

    if (loginBtn) {
        connect(loginBtn, &QPushButton::clicked, this, &LoginPage::onLoginClicked);
        qDebug() << "Login button connected! Object name:" << loginBtn->objectName();
    } else {
        qDebug() << "Login button NOT found!";

        // List all buttons for debugging
        QList<QPushButton*> allButtons = findChildren<QPushButton*>();
        qDebug() << "Available buttons:";
        for (QPushButton *btn : allButtons) {
            qDebug() << "  -" << btn->objectName() << "(" << btn->text() << ")";
        }
    }
}

LoginPage::~LoginPage()
{
    delete ui;
}

void LoginPage::onLoginClicked()
{
    qDebug() << "Login button clicked!";

    QLineEdit *usernameEdit = findChild<QLineEdit*>("lineEditUsername");
    QLineEdit *passwordEdit = findChild<QLineEdit*>("lineEditPassword");

    if (usernameEdit && passwordEdit) {
        QString username = usernameEdit->text();
        QString password = passwordEdit->text();

        qDebug() << "Username:" << username;
        qDebug() << "Password:" << password;

        // Check if credentials match
        if ((username == "admin" && password == "admin123") ||
            (username == "user" && password == "user")) {
            qDebug() << "Login successful!";
            emit loginSuccess();
        } else {
            qDebug() << "Login failed!";
            QMessageBox::warning(this, "Login Failed",
                                 "Invalid username or password!\n\n"
                                 "Demo credentials:\n"
                                 "Username: admin\n"
                                 "Password: admin123");
        }
    } else {
        // If no username/password fields, just login
        qDebug() << "No username/password fields - logging in directly";
        emit loginSuccess();
    }
}