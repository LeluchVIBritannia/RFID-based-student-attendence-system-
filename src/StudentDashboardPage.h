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

private slots:
    void onBuyFoodClicked();
    void onBackToInfoClicked();
    void onPurchaseClicked();

private:
    bool ensureDatabaseOpen();
    void updateUI(const Student &student);
    void updateAttendanceStats(int studentId);
    void updateBalance(int studentId);
    void updateRecentTransactions(int studentId);
    void updateTodayMeals(int studentId);
    void updateFoodPage();

    Ui::StudentDashboardPage *ui;
    DatabaseManager *m_db;

    Student m_currentStudent;
    int     m_currentBalance;
};
#endif