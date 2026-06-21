#ifndef SCANTERMINALPAGE_H
#define SCANTERMINALPAGE_H

#include <QWidget>
#include "database.h"

namespace Ui { class ScanTerminalPage; }

class ScanTerminalPage : public QWidget
{
    Q_OBJECT

public:
    explicit ScanTerminalPage(QWidget *parent = nullptr);
    ~ScanTerminalPage();

    void refreshData();

private slots:
    void onRFIDScanned(const QString &rfidCardId, const QString &mode = "attendance");

private:
    void wireButtons();
    void setState(const QString &icon, const QString &msg, const QString &detail,
                  const QString &color, const QString &borderColor);

    Ui::ScanTerminalPage *ui;
    DatabaseManager *m_db;
};

#endif