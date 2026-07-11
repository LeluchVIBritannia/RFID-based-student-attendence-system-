#ifndef SCANTERMINALPAGE_H
#define SCANTERMINALPAGE_H

#include <QWidget>
#include "database.h"

class SerialManager;  // Forward declaration

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
    void onSerialCardScanned(const QString &uid);  // ADD THIS

private:
    void wireButtons();
    void setState(const QString &icon, const QString &msg, const QString &detail,
                  const QString &color, const QString &borderColor);
    void navigateToStudentDashboard(const QString &rfidCardId);

    Ui::ScanTerminalPage *ui;
    DatabaseManager *m_db;
    SerialManager *m_serialManager;  // ADD THIS
};

#endif