#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QTimer>
#include "database.h"
#include "LoginPage.h"
#include "DashboardPage.h"
#include "StudentDashboardPage.h"
#include "ScanTerminalPage.h"

class SerialManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void showStudentDashboard(const QString &rfidCardId);

public slots:
    void goToDashboard();
    void goToLogin();

private slots:
    void handleIdleTimeout();

private:
    enum PageIndex {
        SCAN_TERMINAL = 0,
        LOGIN = 1,
        DASHBOARD = 2,
        STUDENT_DASHBOARD = 3
    };

    QStackedWidget *m_stack;
    LoginPage *m_loginPage;
    DashboardPage *m_dashboardPage;
    StudentDashboardPage *m_studentDashboardPage;
    ScanTerminalPage *m_scanTerminalPage;

    SerialManager *m_serial;
    DatabaseManager *m_db;
    QTimer *m_idleTimer;
};

#endif