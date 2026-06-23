#include "OverviewPage.h"
#include "ui_OverviewPage.h"

#include <QTableWidgetItem>
#include <QColor>
#include <QLabel>
#include <QDebug>
#include <QDateTime>

static QTableWidgetItem* colorItem(const QString &text, const QString &hex)
{
    auto *item = new QTableWidgetItem(text);
    item->setForeground(QColor(hex));
    return item;
}

OverviewPage::OverviewPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::OverviewPage), m_db(DatabaseManager::instance())
{
    ui->setupUi(this);
    refreshData();
}

OverviewPage::~OverviewPage()
{
    delete ui;
}

void OverviewPage::refreshData()
{
    if (!m_db || !m_db->isOpen()) {
        qDebug() << "Database not available in OverviewPage";
        return;
    }

    qDebug() << "Refreshing Overview page...";
    updateStatistics();
    updateRecentActivity();
}

void OverviewPage::updateStatistics()
{
    if (!m_db || !m_db->isOpen()) return;

    Statistics stats = m_db->getStatistics();
    qDebug() << "Stats from database:";
    qDebug() << "   - Total Students:" << stats.totalStudents;
    qDebug() << "   - Present Today:" << stats.presentToday;
    qDebug() << "   - Absent Today:" << stats.absentToday;
    qDebug() << "   - Transactions:" << stats.transactions;
    qDebug() << "   - Revenue:" << stats.revenue;
    qDebug() << "   - Unique Students:" << stats.uniqueStudents;

    // Update - sc1Value
    QLabel *lblTotalStudents = findChild<QLabel*>("sc1Value");
    if (lblTotalStudents) {
        lblTotalStudents->setText(QString::number(stats.totalStudents));
        qDebug() << "Updated TOTAL STUDENTS to:" << stats.totalStudents;
    } else {
        qDebug() << "sc1Value not found";
    }

    // Update - sc2Value
    QLabel *lblPresentToday = findChild<QLabel*>("sc2Value");
    if (lblPresentToday) {
        lblPresentToday->setText(QString::number(stats.presentToday));
        qDebug() << "Updated PRESENT TODAY to:" << stats.presentToday;
    } else {
        qDebug() << "sc2Value not found";
    }

    // Update - sc3Value
    QLabel *lblAbsentToday = findChild<QLabel*>("sc3Value");
    if (lblAbsentToday) {
        lblAbsentToday->setText(QString::number(stats.absentToday));
        qDebug() << "Updated ABSENT TODAY to:" << stats.absentToday;
    } else {
        qDebug() << "sc3Value not found";
    }

    // Update - sc4Value
    QLabel *lblTransactions = findChild<QLabel*>("sc4Value");
    if (lblTransactions) {
        lblTransactions->setText(QString::number(stats.transactions));
        qDebug() << "Updated TRANSACTIONS to:" << stats.transactions;
    } else {
        qDebug() << "sc4Value not found";
    }
}

void OverviewPage::updateRecentActivity()
{
    if (!m_db || !m_db->isOpen()) return;

    QTableWidget *t = ui->tableOverview;
    if (!t) {
        qDebug() << "tableOverview not found in UI";
        return;
    }

    t->setColumnCount(4);
    t->setHorizontalHeaderLabels({"Student", "Roll No.", "Action", "Time"});
    t->horizontalHeader()->setStretchLastSection(true);


    QList<AttendanceRecord> attendance = m_db->getTodaysAttendance();
    qDebug() << "Recent activity has" << attendance.size() << "entries";

    // Clear existing rows
    t->setRowCount(0);

    if (attendance.isEmpty()) {
        t->setRowCount(1);
        QTableWidgetItem *msgItem = new QTableWidgetItem("No recent activity");
        msgItem->setTextAlignment(Qt::AlignCenter);
        t->setItem(0, 0, msgItem);
        t->setSpan(0, 0, 1, 4);
        return;
    }

    int rowCount = qMin(5, attendance.size());
    t->setRowCount(rowCount);

    for (int r = 0; r < rowCount; ++r) {
        const AttendanceRecord &record = attendance[r];

        t->setItem(r, 0, new QTableWidgetItem(record.studentName));
        t->setItem(r, 1, new QTableWidgetItem(record.rollNo));
        t->setItem(r, 2, colorItem("Attendance", "#2ECC71"));
        t->setItem(r, 3, new QTableWidgetItem(record.timeIn));
        t->setRowHeight(r, 40);
    }
}