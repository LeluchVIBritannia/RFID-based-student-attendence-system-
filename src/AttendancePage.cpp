#include "AttendancePage.h"
#include "ui_AttendancePage.h"

#include <QTableWidgetItem>
#include <QColor>
#include <QLabel>
#include <QDebug>
#include <QMessageBox>

static QTableWidgetItem* colorItem(const QString &text, const QString &hex)
{
    auto *item = new QTableWidgetItem(text);
    item->setForeground(QColor(hex));
    return item;
}

AttendancePage::AttendancePage(QWidget *parent)
    : QWidget(parent), ui(new Ui::AttendancePage), m_db(DatabaseManager::instance())
{
    ui->setupUi(this);
    refreshData();
}

AttendancePage::~AttendancePage()
{
    delete ui;
}

void AttendancePage::refreshData()
{
    if (!m_db) return;
    updateAttendanceTable();
}

void AttendancePage::onRFIDScanned(const QString &rfidCardId)
{
    if (!m_db) {
        QMessageBox::warning(this, "Error", "Database not available");
        return;
    }

    Student student = m_db->getStudentByRFID(rfidCardId);

    if (student.id == -1) {
        QMessageBox::warning(this, "Invalid Card", "Card not recognized!");
        return;
    }

    bool success = m_db->recordAttendance(student.id, "Morning");

    if (success) {
        QMessageBox::information(this, "Success",
                                 QString("Attendance recorded for %1").arg(student.name));
        refreshData();
    } else {
        QMessageBox::warning(this, "Already Recorded",
                             QString("%1 already recorded for today").arg(student.name));
    }
}

void AttendancePage::updateAttendanceTable()
{
    if (!m_db) return;

    QTableWidget *t = ui->tableAttendance;
    if (!t) return;

    t->setColumnCount(5);
    t->setHorizontalHeaderLabels({"Name", "Roll No.", "Status", "Time In", "Sessions"});
    t->horizontalHeader()->setStretchLastSection(true);

    QList<AttendanceRecord> attendance = m_db->getTodaysAttendance();
    t->setRowCount(0);

    if (attendance.isEmpty()) {
        t->setRowCount(1);
        QTableWidgetItem *msgItem = new QTableWidgetItem("No attendance records for today");
        msgItem->setTextAlignment(Qt::AlignCenter);
        t->setItem(0, 0, msgItem);
        t->setSpan(0, 0, 1, 5);
        return;
    }

    t->setRowCount(attendance.size());

    for (int r = 0; r < attendance.size(); ++r) {
        const AttendanceRecord &record = attendance[r];
        t->setItem(r, 0, new QTableWidgetItem(record.studentName));
        t->setItem(r, 1, new QTableWidgetItem(record.rollNo));
        t->setItem(r, 2, colorItem(record.status,
                                   record.status == "Present" ? "#2ECC71" : "#E74C3C"));
        t->setItem(r, 3, new QTableWidgetItem(record.timeIn));
        t->setItem(r, 4, new QTableWidgetItem(record.session));
        t->setRowHeight(r, 40);
    }
}