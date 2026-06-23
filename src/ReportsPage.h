#ifndef REPORTSPAGE_H
#define REPORTSPAGE_H

#include <QWidget>
#include "database.h"

namespace Ui { class ReportsPage; }

class ReportsPage : public QWidget
{
    Q_OBJECT

public:
    explicit ReportsPage(QWidget *parent = nullptr);
    ~ReportsPage();

    void refreshData();

private:
    void updateStatisticsCards();
    void updateAttendanceReport();
    void updateExpenditureReport();

    Ui::ReportsPage *ui;
    DatabaseManager *m_db;
};

#endif