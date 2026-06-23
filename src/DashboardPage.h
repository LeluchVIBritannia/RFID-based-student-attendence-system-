#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include <QWidget>
#include "database.h"

namespace Ui { class DashboardPage; }

class OverviewPage;
class AttendancePage;
class CafeteriaPage;
class StudentsPage;
class ReportsPage;
class RegisterCardPage;
class ScanTerminalPage;

class DashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);
    ~DashboardPage();

    void setDatabase(DatabaseManager *db);
    void refreshAllPages();
    void navigateToRegisterPage();

signals:
    void logoutRequested();
private slots:
    void onNavClicked();

private:
    void wireNav();
    void setupPages();
    void updateOverviewPage();
    void updateAttendancePage();
    void updateCafeteriaPage();
    void updateStudentsPage();
    void updateReportsPage();
    void updateRegisterCardPage();
    void updateScanTerminalPage();

    Ui::DashboardPage *ui;
    DatabaseManager *m_db;

    OverviewPage     *m_overviewPage;
    AttendancePage   *m_attendancePage;
    CafeteriaPage    *m_cafeteriaPage;
    StudentsPage     *m_studentsPage;
    ReportsPage      *m_reportsPage;
    RegisterCardPage *m_registerPage;
    ScanTerminalPage *m_scanPage;
};

#endif