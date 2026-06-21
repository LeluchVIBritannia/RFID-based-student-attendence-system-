#ifndef CAFETERIAPAGE_H
#define CAFETERIAPAGE_H

#include <QWidget>
#include "database.h"

namespace Ui { class CafeteriaPage; }

class CafeteriaPage : public QWidget
{
    Q_OBJECT

public:
    explicit CafeteriaPage(QWidget *parent = nullptr);
    ~CafeteriaPage();

    void refreshData();

public slots:
    void onRFIDScanned(const QString &rfidCardId);
    void onPurchaseItem(const QString &item, int amount);

private:
    void updateStatistics();
    void updateTransactionLog();

    Ui::CafeteriaPage *ui;
    DatabaseManager *m_db;
};

#endif