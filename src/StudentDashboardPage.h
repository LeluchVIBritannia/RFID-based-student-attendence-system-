#ifndef STUDENTDASHBOARDPAGE_H
#define STUDENTDASHBOARDPAGE_H

#include <QWidget>
#include "database.h"

namespace Ui { class StudentDashboardPage; }

class StudentDashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit StudentDashboardPage(QWidget *parent = nullptr);
    ~StudentDashboardPage();

    void loadStudentByCardId(const QString &cardId);
    void refreshData();

private:
    bool ensureDatabaseOpen();
    void updateUI(const Student &student);
    void updateAttendanceStats(int studentId);
    void updateBalance(int studentId);
    void updateRecentTransactions(int studentId);
    void updateTodayMeals(int studentId);

    // Helper to execute queries safely
    bool executeQuery(QSqlQuery &query, const QString &sql);
    bool bindAndExecute(QSqlQuery &query);

    Ui::StudentDashboardPage *ui;
    DatabaseManager *m_db;
    QSqlDatabase m_dbConnection;  // Keep a separate connection
};

#endif