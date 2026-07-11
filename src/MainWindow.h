#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QStackedWidget>
#include <QTimer>
#include "database.h"

class LoginPage;
class DashboardPage;
class StudentDashboardPage;
class SerialManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    enum Page { LOGIN = 0, DASHBOARD = 1, STUDENT_VIEW = 2 };
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void goToDashboard();
    void goToLogin();
    void handleCardScanned(const QString &uid);
    void handleIdleTimeout();

private:
    QStackedWidget       *m_stack;
    LoginPage            *m_loginPage;
    DashboardPage        *m_dashboardPage;
    StudentDashboardPage *m_studentViewPage;
    SerialManager        *m_serial;
    QTimer               *m_idleTimer;
    DatabaseManager      *m_db;
};
#endif