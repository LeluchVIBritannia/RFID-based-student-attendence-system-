#ifndef STUDENTDASHBOARDPAGE_H
#define STUDDENTDASHBOARDPAGE_H

#include <QWidget>
#include "database.h"

namespace Ui { class StudentDashboardPage; }

class StudentDashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit StudentDashboardPage(QWidget *parent = nullptr);
    ~StudentDashboardPage();

    // Call this when a card is tapped/scanned to refresh the view with
    // that student's live data from the database
    void loadStudentByCardId(const QString &cardId);
    void refreshData();

private:
    void updateUI(const Student &student);
    void updateAttendanceStats(int studentId);
    void updateBalance(int studentId);
    void updateRecentTransactions(int studentId);
    void updateTodayMeals(int studentId);

    Ui::StudentDashboardPage *ui;
    DatabaseManager *m_db;
};

#endif