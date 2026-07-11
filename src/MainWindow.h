#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include "database.h"

class LoginPage;
class DashboardPage;
class StudentDashboardPage;  // ADD THIS

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum Page { LOGIN = 0, DASHBOARD = 1, STUDENT_DASHBOARD = 2 };  // ADD STUDENT_DASHBOARD

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Public method to show student dashboard when RFID is scanned
    void showStudentDashboard(const QString &rfidCardId);

private slots:
    void goToDashboard();
    void goToLogin();

private:
    QStackedWidget      *m_stack;
    LoginPage           *m_loginPage;
    DashboardPage       *m_dashboardPage;
    StudentDashboardPage *m_studentDashboardPage;  // ADD THIS
    DatabaseManager     *m_db;
};

#endif