#ifndef OVERVIEWPAGE_H
#define OVERVIEWPAGE_H

#include <QWidget>
#include "database.h"

namespace Ui { class OverviewPage; }

class OverviewPage : public QWidget
{
    Q_OBJECT

public:
    explicit OverviewPage(QWidget *parent = nullptr);
    ~OverviewPage();

    void refreshData();

private:
    void updateStatistics();
    void updateRecentActivity();

    Ui::OverviewPage *ui;
    DatabaseManager *m_db;
};

#endif