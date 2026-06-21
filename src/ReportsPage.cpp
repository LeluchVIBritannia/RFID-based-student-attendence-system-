#include "ReportsPage.h"
#include "ui_ReportsPage.h"

#include <QTableWidgetItem>
#include <QColor>
#include <QLabel>
#include <QDebug>
#include <QDateTime>
#include <QSqlQuery>

static QTableWidgetItem* colorItem(const QString &text, const QString &hex)
{
    auto *item = new QTableWidgetItem(text);
    item->setForeground(QColor(hex));
    return item;
}

ReportsPage::ReportsPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::ReportsPage), m_db(DatabaseManager::instance())
{
    ui->setupUi(this);
    refreshData();
}

ReportsPage::~ReportsPage()
{
    delete ui;
}

void ReportsPage::refreshData()
{
    if (!m_db || !m_db->isOpen()) {
        qDebug() << "Database not available in ReportsPage";
        return;
    }

    qDebug() << "Refreshing Reports page...";

    QLabel *dateLabel = findChild<QLabel*>("labelDate");
    if (dateLabel) {
        dateLabel->setText(QDateTime::currentDateTime().toString("ddd, dd MMM yyyy"));
    }

    updateStatisticsCards();
    updateAttendanceReport();
    updateExpenditureReport();
}

void ReportsPage::updateStatisticsCards()
{
    if (!m_db || !m_db->isOpen()) return;

    Statistics stats = m_db->getStatistics();
    qDebug() << "Stats from database:";
    qDebug() << "   - Total Students:" << stats.totalStudents;
    qDebug() << "   - Present Today:" << stats.presentToday;
    qDebug() << "   - Avg Attendance:" << stats.avgAttendance;
    qDebug() << "   - Monthly Spend:" << stats.totalMonthlySpend;
    qDebug() << "   - Top Attendee:" << stats.topAttendee;

    // Update - repVal1
    QLabel *lblAvgAttendance = findChild<QLabel*>("repVal1");
    if (lblAvgAttendance) {
        if (stats.totalStudents > 0 && stats.avgAttendance > 0) {
            lblAvgAttendance->setText(QString("%1%").arg(stats.avgAttendance, 0, 'f', 1));
        } else {
            lblAvgAttendance->setText("0%");
        }
        qDebug() << "Updated AVG. ATTENDANCE to:" << lblAvgAttendance->text();
    }

    // Update - repVal2
    QLabel *lblMonthlySpend = findChild<QLabel*>("repVal2");
    if (lblMonthlySpend) {
        if (stats.totalMonthlySpend > 0) {
            lblMonthlySpend->setText(QString("%1").arg(stats.totalMonthlySpend));
        } else {
            lblMonthlySpend->setText("0");
        }
        qDebug() << "Updated TOTAL MONTHLY SPEND to:" << lblMonthlySpend->text();
    }

    // Update - repVal3
    QLabel *lblTopAttendee = findChild<QLabel*>("repVal3");
    if (lblTopAttendee) {
        if (!stats.topAttendee.isEmpty() && stats.topAttendee != "No data" && stats.topAttendee != "N/A") {
            lblTopAttendee->setText(stats.topAttendee);
        } else {
            lblTopAttendee->setText("No data");
        }
        qDebug() << "Updated TOP ATTENDEE to:" << lblTopAttendee->text();
    }
}

void ReportsPage::updateAttendanceReport()
{
    if (!m_db || !m_db->isOpen()) return;

    QTableWidget *t = ui->tableAttendanceReport;
    if (!t) {
        qDebug() << "tableAttendanceReport not found in UI";
        return;
    }

    QList<QMap<QString, QVariant>> report = m_db->getAttendanceReport();

    t->setColumnCount(5);
    t->setHorizontalHeaderLabels({"Name", "Roll No.", "Classes Attended", "Total Classes", "Attendance %"});
    t->horizontalHeader()->setStretchLastSection(true);

    t->setRowCount(0);

    if (report.isEmpty()) {
        t->setRowCount(1);
        QTableWidgetItem *msgItem = new QTableWidgetItem("No attendance records found");
        msgItem->setTextAlignment(Qt::AlignCenter);
        t->setItem(0, 0, msgItem);
        t->setSpan(0, 0, 1, 5);
        return;
    }

    int totalClasses = 0;
    QSqlQuery totalQuery(m_db->getDatabase());
    totalQuery.prepare("SELECT COUNT(DISTINCT date) FROM attendance");
    if (totalQuery.exec() && totalQuery.next()) {
        totalClasses = totalQuery.value(0).toInt();
    }
    qDebug() << "Total classes for report:" << totalClasses;

    t->setRowCount(report.size());

    for (int r = 0; r < report.size(); ++r) {
        const auto &row = report[r];
        QString name = row["name"].toString();
        QString rollNo = row["roll_no"].toString();
        int attended = row["attended"].toInt();
        int percentage = (totalClasses > 0) ? (attended * 100 / totalClasses) : 0;

        bool good = percentage >= 75;

        t->setItem(r, 0, new QTableWidgetItem(name));
        t->setItem(r, 1, new QTableWidgetItem(rollNo));
        t->setItem(r, 2, new QTableWidgetItem(QString::number(attended)));
        t->setItem(r, 3, new QTableWidgetItem(QString::number(totalClasses)));
        t->setItem(r, 4, colorItem(QString("%1%").arg(percentage), good ? "#2ECC71" : "#E74C3C"));
        t->setRowHeight(r, 40);
    }
}

void ReportsPage::updateExpenditureReport()
{
    if (!m_db || !m_db->isOpen()) return;

    QTableWidget *t = ui->tableExpenditureReport;
    if (!t) {
        qDebug() << "tableExpenditureReport not found in UI";
        return;
    }

    QList<QMap<QString, QVariant>> report = m_db->getCafeteriaReport();
    qDebug() << "Cafeteria report has" << report.size() << "entries";

    t->setColumnCount(5);
    t->setHorizontalHeaderLabels({"Name", "Roll No.", "Transactions", "Total Spent (NPR)", "Remaining Balance (NPR)"});
    t->horizontalHeader()->setStretchLastSection(true);

    t->setRowCount(0);

    if (report.isEmpty()) {
        t->setRowCount(1);
        QTableWidgetItem *msgItem = new QTableWidgetItem("No cafeteria transactions found");
        msgItem->setTextAlignment(Qt::AlignCenter);
        t->setItem(0, 0, msgItem);
        t->setSpan(0, 0, 1, 5);
        return;
    }

    t->setRowCount(report.size());

    for (int r = 0; r < report.size(); ++r) {
        const auto &row = report[r];
        QString name = row["name"].toString();
        QString rollNo = row["roll_no"].toString();
        int transactions = row["transactions"].toInt();
        int totalSpent = row["total_spent"].toInt();
        int balance = row["balance"].toInt();

        t->setItem(r, 0, new QTableWidgetItem(name));
        t->setItem(r, 1, new QTableWidgetItem(rollNo));
        t->setItem(r, 2, new QTableWidgetItem(QString::number(transactions)));
        t->setItem(r, 3, colorItem(QString("%1").arg(totalSpent), "#F39C12"));
        t->setItem(r, 4, colorItem(QString("%1").arg(balance), "#2ECC71"));
        t->setRowHeight(r, 40);
    }
}