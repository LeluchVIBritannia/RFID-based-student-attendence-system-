#include "StudentDashboardPage.h"
#include "ui_StudentDashboardPage.h"

#include <QDebug>
#include <QDateTime>
#include <QSqlQuery>

StudentDashboardPage::StudentDashboardPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StudentDashboardPage)
    , m_db(DatabaseManager::instance())
{
    ui->setupUi(this);

    // Set default avatar initials
    ui->labelAvatar->setText("??");

    // Show waiting state
    ui->labelFooterText->setText("Waiting for card tap...");

    // Ensure database is open
    if (m_db && !m_db->isOpen()) {
        qDebug() << "⚠️ Database not open, reinitializing...";
        m_db->initDatabase();
    }
}

StudentDashboardPage::~StudentDashboardPage()
{
    delete ui;
}

// Helper function to ensure database is open
bool StudentDashboardPage::ensureDatabaseOpen()
{
    if (!m_db) {
        qDebug() << "❌ Database manager is null!";
        return false;
    }

    if (!m_db->isOpen()) {
        qDebug() << "⚠️ Database is not open, attempting to reopen...";
        m_db->initDatabase();
        if (!m_db->isOpen()) {
            qDebug() << "❌ Failed to reopen database!";
            return false;
        }
        qDebug() << "✅ Database reopened successfully";
    }
    return true;
}

void StudentDashboardPage::refreshData()
{
    if (!ensureDatabaseOpen()) {
        ui->labelFooterText->setText("❌ Database connection failed");
        return;
    }

    // If there's a current student, reload their data
    // For now, just show waiting state
    ui->labelAvatar->setText("??");
    ui->labelStudentName->setText("Tap your RFID card");
    ui->labelStudentMeta->setText("to view your dashboard");
    ui->labelCardStatus->setText("⏳ Waiting...");

    ui->sc1Value->setText("—");
    ui->sc1Sub->setText("");
    ui->sc2Value->setText("—");
    ui->sc2Sub->setText("");
    ui->sc3Value->setText("—");
    ui->sc3Sub->setText("");

    ui->mealItem1->setText("—");
    ui->mealTime1->setText("");
    ui->mealItem2->setText("—");
    ui->mealTime2->setText("");
    ui->labelMealFooter->setText("No purchases today");

    ui->transName1->setText("—");
    ui->transDate1->setText("");
    ui->transAmt1->setText("");
    ui->transName2->setText("—");
    ui->transDate2->setText("");
    ui->transAmt2->setText("");
    ui->transName3->setText("—");
    ui->transDate3->setText("");
    ui->transAmt3->setText("");

    ui->labelFooterText->setText("Tap your RFID card to view your dashboard");
}

void StudentDashboardPage::loadStudentByCardId(const QString &cardId)
{
    qDebug() << "📱 Loading student dashboard for RFID:" << cardId;

    // Ensure database is open
    if (!ensureDatabaseOpen()) {
        ui->labelAvatar->setText("❌");
        ui->labelStudentName->setText("Database error");
        ui->labelStudentMeta->setText("Please contact administrator");
        ui->labelCardStatus->setText("❌ DB Error");
        ui->labelFooterText->setText("❌ Database connection failed");
        return;
    }

    // Get student by RFID
    Student student = m_db->getStudentByRFID(cardId);

    if (student.id == -1) {
        qDebug() << "❌ Student not found for RFID:" << cardId;
        ui->labelAvatar->setText("❌");
        ui->labelStudentName->setText("Card not recognized");
        ui->labelStudentMeta->setText("Please contact the administrator");
        ui->labelCardStatus->setText("❌ Invalid card");
        ui->labelFooterText->setText("❌ Card ID " + cardId + " not recognized");
        return;
    }

    qDebug() << "✅ Student found:" << student.name << "(ID:" << student.id << ")";

    // Update UI with student data
    updateUI(student);
}

void StudentDashboardPage::updateUI(const Student &student)
{
    // 1. Set avatar initials
    QStringList nameParts = student.name.split(" ");
    QString initials;
    if (nameParts.size() >= 2) {
        initials = nameParts[0].left(1).toUpper() + nameParts[nameParts.size()-1].left(1).toUpper();
    } else if (nameParts.size() == 1) {
        initials = nameParts[0].left(2).toUpper();
    } else {
        initials = "??";
    }
    ui->labelAvatar->setText(initials);

    // 2. Student info
    ui->labelStudentName->setText(student.name);
    ui->labelStudentMeta->setText(QString("Roll No. %1  ·  %2  ·  Kathmandu University")
                                      .arg(student.rollNo)
                                      .arg(student.classSection));
    ui->labelCardStatus->setText("✓  Card active");
    ui->labelCardStatus->setStyleSheet("background-color:#1E3D2B; color:#2ECC71; border-radius:15px; padding:0 16px; font-size:13px; font-weight:600;");

    // 3. Update attendance stats
    updateAttendanceStats(student.id);

    // 4. Update balance
    updateBalance(student.id);

    // 5. Update recent transactions
    updateRecentTransactions(student.id);

    // 6. Update today's meals
    updateTodayMeals(student.id);

    // 7. Footer
    ui->labelFooterText->setText(QString("Card ID %1 · Tap your card again anytime to refresh this view")
                                     .arg(student.rfidCardId));
}

void StudentDashboardPage::updateAttendanceStats(int studentId)
{
    if (!ensureDatabaseOpen()) return;

    // Get total classes (distinct dates)
    int totalClasses = 0;
    QSqlQuery totalQuery(m_db->getDatabase());
    totalQuery.prepare("SELECT COUNT(DISTINCT date) FROM attendance");
    if (totalQuery.exec() && totalQuery.next()) {
        totalClasses = totalQuery.value(0).toInt();
    } else {
        qDebug() << "❌ Failed to get total classes:" << totalQuery.lastError().text();
    }

    // Get attended classes for this student
    int attended = 0;
    QSqlQuery attQuery(m_db->getDatabase());
    attQuery.prepare("SELECT COUNT(*) FROM attendance WHERE student_id = ?");
    attQuery.addBindValue(studentId);
    if (attQuery.exec() && attQuery.next()) {
        attended = attQuery.value(0).toInt();
    } else {
        qDebug() << "❌ Failed to get attendance for student:" << attQuery.lastError().text();
    }

    // Get today's attendance status
    QString todayStatus = "Not marked";
    QString todayTime = "";
    QSqlQuery todayQuery(m_db->getDatabase());
    todayQuery.prepare("SELECT time_in FROM attendance WHERE student_id = ? AND date = ?");
    todayQuery.addBindValue(studentId);
    todayQuery.addBindValue(QDate::currentDate().toString("yyyy-MM-dd"));
    if (todayQuery.exec() && todayQuery.next()) {
        todayStatus = "Marked";
        todayTime = todayQuery.value(0).toString();
    }

    // Calculate percentage
    int percentage = (totalClasses > 0) ? (attended * 100 / totalClasses) : 0;

    // Update UI
    if (todayStatus == "Marked") {
        ui->sc1Value->setText("✅ Marked");
        ui->sc1Value->setStyleSheet("color:#2ECC71; font-size:24px; font-weight:700; background:transparent; border:none;");
        ui->sc1Sub->setText(todayTime);
    } else {
        ui->sc1Value->setText("❌ Not marked");
        ui->sc1Value->setStyleSheet("color:#E74C3C; font-size:24px; font-weight:700; background:transparent; border:none;");
        ui->sc1Sub->setText("Please mark attendance");
    }

    ui->sc2Value->setText(QString("%1%").arg(percentage));
    ui->sc2Sub->setText(QString("%1 of %2 classes").arg(attended).arg(totalClasses));

    // Color based on percentage
    if (percentage >= 75) {
        ui->sc2Value->setStyleSheet("color:#2ECC71; font-size:24px; font-weight:700; background:transparent; border:none;");
    } else if (percentage >= 50) {
        ui->sc2Value->setStyleSheet("color:#F39C12; font-size:24px; font-weight:700; background:transparent; border:none;");
    } else {
        ui->sc2Value->setStyleSheet("color:#E74C3C; font-size:24px; font-weight:700; background:transparent; border:none;");
    }
}

void StudentDashboardPage::updateBalance(int studentId)
{
    if (!ensureDatabaseOpen()) return;

    int balance = m_db->getStudentBalance(studentId);
    ui->sc3Value->setText(QString("Rs. %1").arg(balance));
    ui->sc3Sub->setText(QString("Balance updated %1").arg(QDate::currentDate().toString("MMM dd")));

    // Color based on balance
    if (balance > 500) {
        ui->sc3Value->setStyleSheet("color:#2ECC71; font-size:24px; font-weight:700; background:transparent; border:none;");
    } else if (balance > 100) {
        ui->sc3Value->setStyleSheet("color:#F39C12; font-size:24px; font-weight:700; background:transparent; border:none;");
    } else {
        ui->sc3Value->setStyleSheet("color:#E74C3C; font-size:24px; font-weight:700; background:transparent; border:none;");
    }
}

void StudentDashboardPage::updateRecentTransactions(int studentId)
{
    if (!ensureDatabaseOpen()) return;

    QList<CafeteriaTransaction> transactions = m_db->getTransactionsByStudent(studentId);

    // Clear existing transaction rows
    ui->transName1->setText("—");
    ui->transDate1->setText("");
    ui->transAmt1->setText("");
    ui->transName2->setText("—");
    ui->transDate2->setText("");
    ui->transAmt2->setText("");
    ui->transName3->setText("—");
    ui->transDate3->setText("");
    ui->transAmt3->setText("");

    // Show up to 3 most recent transactions
    int count = qMin(3, transactions.size());

    if (count > 0) {
        ui->transName1->setText(transactions[0].item);
        ui->transDate1->setText(transactions[0].date + ", " + transactions[0].time);
        ui->transAmt1->setText("- Rs. " + QString::number(transactions[0].amount));
        ui->transAmt1->setStyleSheet("color:#E8E8F0; font-size:14px; font-weight:600; background:transparent; border:none;");
    }

    if (count > 1) {
        ui->transName2->setText(transactions[1].item);
        ui->transDate2->setText(transactions[1].date + ", " + transactions[1].time);
        ui->transAmt2->setText("- Rs. " + QString::number(transactions[1].amount));
        ui->transAmt2->setStyleSheet("color:#E8E8F0; font-size:14px; font-weight:600; background:transparent; border:none;");
    }

    if (count > 2) {
        ui->transName3->setText(transactions[2].item);
        ui->transDate3->setText(transactions[2].date + ", " + transactions[2].time);
        ui->transAmt3->setText("- Rs. " + QString::number(transactions[2].amount));
        ui->transAmt3->setStyleSheet("color:#E8E8F0; font-size:14px; font-weight:600; background:transparent; border:none;");
    }
}

void StudentDashboardPage::updateTodayMeals(int studentId)
{
    if (!ensureDatabaseOpen()) return;

    QDate today = QDate::currentDate();
    QString todayStr = today.toString("yyyy-MM-dd");

    // Get today's transactions
    QList<CafeteriaTransaction> todayTransactions;
    QSqlQuery query(m_db->getDatabase());
    query.prepare("SELECT item, amount, time FROM cafeteria_transactions "
                  "WHERE student_id = ? AND date = ? "
                  "ORDER BY time DESC");
    query.addBindValue(studentId);
    query.addBindValue(todayStr);

    if (query.exec()) {
        while (query.next()) {
            CafeteriaTransaction trans;
            trans.item = query.value("item").toString();
            trans.amount = query.value("amount").toInt();
            trans.time = query.value("time").toString();
            todayTransactions.append(trans);
        }
    } else {
        qDebug() << "❌ Failed to get today's meals:" << query.lastError().text();
    }

    // Update meal items
    int totalSpent = 0;

    if (todayTransactions.size() > 0) {
        ui->mealItem1->setText(todayTransactions[0].item);
        ui->mealTime1->setText(todayTransactions[0].time);
        totalSpent += todayTransactions[0].amount;
    } else {
        ui->mealItem1->setText("—");
        ui->mealTime1->setText("");
    }

    if (todayTransactions.size() > 1) {
        ui->mealItem2->setText(todayTransactions[1].item);
        ui->mealTime2->setText(todayTransactions[1].time);
        totalSpent += todayTransactions[1].amount;
    } else {
        ui->mealItem2->setText("—");
        ui->mealTime2->setText("");
    }

    if (todayTransactions.size() == 0) {
        ui->labelMealFooter->setText("No purchases today");
    } else {
        ui->labelMealFooter->setText(QString("%1 purchases · Rs. %2 spent today")
                                         .arg(todayTransactions.size())
                                         .arg(totalSpent));
    }
}