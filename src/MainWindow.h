#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include "database.h"

class LoginPage;
class DashboardPage;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum Page { LOGIN = 0, DASHBOARD = 1 };
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void goToDashboard();
    void goToLogin();

private:
    QStackedWidget  *m_stack;
    LoginPage       *m_loginPage;
    DashboardPage   *m_dashboardPage;
    DatabaseManager *m_db;
};

#endif