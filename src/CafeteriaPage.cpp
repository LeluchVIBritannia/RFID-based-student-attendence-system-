#include "CafeteriaPage.h"
#include "ui_CafeteriaPage.h"

#include <QTableWidgetItem>
#include <QColor>
#include <QLabel>
#include <QDebug>
#include <QDateTime>
#include <QMessageBox>
#include <QPushButton>
#include <QInputDialog>

static QTableWidgetItem* colorItem(const QString &text, const QString &hex)
{
    auto *item = new QTableWidgetItem(text);
    item->setForeground(QColor(hex));
    return item;
}

CafeteriaPage::CafeteriaPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::CafeteriaPage), m_db(DatabaseManager::instance())
{
    ui->setupUi(this);
    refreshData();
}

CafeteriaPage::~CafeteriaPage()
{
    delete ui;
}

void CafeteriaPage::refreshData()
{
    if (!m_db || !m_db->isOpen()) {
        qDebug() << "Database not available in CafeteriaPage";
        return;
    }

    qDebug() << "Refreshing Cafeteria page...";
    updateStatistics();
    updateTransactionLog();
}

void CafeteriaPage::updateStatistics()
{
    if (!m_db || !m_db->isOpen()) return;

    Statistics stats = m_db->getStatistics();
    qDebug() << "Stats - Transactions:" << stats.transactions
             << "Revenue:" << stats.revenue
             << "Unique:" << stats.uniqueStudents;

    // Update TODAY'S TRANSACTIONS - cafVal1
    QLabel *lblTransactions = findChild<QLabel*>("cafVal1");
    if (lblTransactions) {
        lblTransactions->setText(QString::number(stats.transactions));
        qDebug() << "Updated TRANSACTIONS to:" << stats.transactions;
    }

    // Update REVENUE TODAY - cafVal2
    QLabel *lblRevenue = findChild<QLabel*>("cafVal2");
    if (lblRevenue) {
        lblRevenue->setText(QString::number(stats.revenue));
        qDebug() << "Updated REVENUE to:" << stats.revenue;
    }

    // Update UNIQUE STUDENTS - cafVal3
    QLabel *lblUniqueStudents = findChild<QLabel*>("cafVal3");
    if (lblUniqueStudents) {
        lblUniqueStudents->setText(QString::number(stats.uniqueStudents));
        qDebug() << "Updated UNIQUE STUDENTS to:" << stats.uniqueStudents;
    }
}

void CafeteriaPage::updateTransactionLog()
{
    if (!m_db || !m_db->isOpen()) return;

    QTableWidget *t = ui->tableCafeteria;
    if (!t) {
        qDebug() << "tableCafeteria not found in UI";
        return;
    }

    // Set column headers
    t->setColumnCount(5);
    t->setHorizontalHeaderLabels({"Student", "Roll No.", "Item", "Amount (NPR)", "Time"});
    t->horizontalHeader()->setStretchLastSection(true);

    // Get today's transactions
    QList<CafeteriaTransaction> transactions = m_db->getTodaysTransactions();
    qDebug() << "Today's transactions has" << transactions.size() << "entries";

    // Clear existing rows
    t->setRowCount(0);

    if (transactions.isEmpty()) {
        t->setRowCount(1);
        QTableWidgetItem *msgItem = new QTableWidgetItem("No cafeteria transactions for today");
        msgItem->setTextAlignment(Qt::AlignCenter);
        t->setItem(0, 0, msgItem);
        t->setSpan(0, 0, 1, 5);
        return;
    }

    t->setRowCount(transactions.size());

    for (int r = 0; r < transactions.size(); ++r) {
        const CafeteriaTransaction &record = transactions[r];

        t->setItem(r, 0, new QTableWidgetItem(record.studentName));
        t->setItem(r, 1, new QTableWidgetItem(record.rollNo));
        t->setItem(r, 2, new QTableWidgetItem(record.item));
        t->setItem(r, 3, colorItem(QString::number(record.amount), "#F39C12"));
        t->setItem(r, 4, new QTableWidgetItem(record.time));
        t->setRowHeight(r, 40);
    }
}

void CafeteriaPage::onRFIDScanned(const QString &rfidCardId)
{
    if (!m_db || !m_db->isOpen()) {
        QMessageBox::warning(this, "Error", "Database not available");
        return;
    }

    Student student = m_db->getStudentByRFID(rfidCardId);

    if (student.id == -1) {
        QMessageBox::warning(this, "Invalid Card", "Card not recognized!");
        return;
    }

    int balance = m_db->getStudentBalance(student.id);

    // Show student info and ask for purchase
    QStringList items = {"Lunch Set (Rs. 180)", "Tea + Snack (Rs. 65)", "Coffee (Rs. 80)",
                         "Pasta (Rs. 120)", "Pizza (Rs. 150)", "Sandwich (Rs. 100)", "Juice (Rs. 50)"};

    bool ok;
    QString selected = QInputDialog::getItem(this, "Select Item",
                                             QString("Student: %1 (Roll: %2)\nBalance: Rs. %3\n\nSelect item to purchase:")
                                                 .arg(student.name)
                                                 .arg(student.rollNo)
                                                 .arg(balance),
                                             items, 0, false, &ok);

    if (ok && !selected.isEmpty()) {
        // Extract item name and amount
        QStringList parts = selected.split(" (Rs. ");
        QString itemName = parts[0];
        int amount = parts[1].replace(")", "").toInt();

        if (balance < amount) {
            QMessageBox::warning(this, "Insufficient Balance",
                                 QString("Insufficient balance!\n\n"
                                         "Item: %1\nAmount: Rs. %2\nYour Balance: Rs. %3")
                                     .arg(itemName).arg(amount).arg(balance));
            return;
        }

        bool success = m_db->recordCafeteriaTransaction(student.id, itemName, amount);

        if (success) {
            QMessageBox::information(this, "Success",
                                     QString("Purchase successful!\n\n"
                                             "Item: %1\nAmount: Rs. %2\nRemaining Balance: Rs. %3")
                                         .arg(itemName).arg(amount).arg(balance - amount));
            refreshData();
        } else {
            QMessageBox::critical(this, "Error", "Transaction failed. Please try again.");
        }
    }
}

void CafeteriaPage::onPurchaseItem(const QString &item, int amount)
{
    QMessageBox::information(this, "Purchase",
                             QString("Item: %1\nAmount: Rs. %2").arg(item).arg(amount));
}