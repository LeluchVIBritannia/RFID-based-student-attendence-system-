#ifndef ATTENDANCEPAGE_H
#define ATTENDANCEPAGE_H

#include <QWidget>
#include "database.h"

namespace Ui { class AttendancePage; }

class AttendancePage : public QWidget
{
    Q_OBJECT

public:
    explicit AttendancePage(QWidget *parent = nullptr);
    ~AttendancePage();

    void refreshData();

    void onRFIDScanned(const QString &rfidCardId);

private:
    void updateAttendanceTable();

    Ui::AttendancePage *ui;
    DatabaseManager *m_db;
};

#endif