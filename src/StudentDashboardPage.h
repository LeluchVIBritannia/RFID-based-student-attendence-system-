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
    bool ensureDatabaseOpen();  // ADD THIS
    void updateUI(const Student &student);
    void updateAttendanceStats(int studentId);
    void updateBalance(int studentId);
    void updateRecentTransactions(int studentId);
    void updateTodayMeals(int studentId);

    Ui::StudentDashboardPage *ui;
    DatabaseManager *m_db;
};

#endif