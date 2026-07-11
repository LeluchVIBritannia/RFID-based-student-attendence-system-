#include "StudentDashboardPage.h"
#include "ui_StudentDashboardPage.h"

#include <QDebug>
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlError>
#include <QCoreApplication>
#include <QDir>

StudentDashboardPage::StudentDashboardPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StudentDashboardPage)
    , m_db(DatabaseManager::instance())
{
    ui->setupUi(this);

    // Create a separate database connection for this page
    m_dbConnection = QSqlDatabase::addDatabase("QSQLITE", "student_dashboard_connection");
    QString dbPath = QDir::currentPath() + "/ku_rfid.db";
    m_dbConnection.setDatabaseName(dbPath);

    if (!m_dbConnection.open()) {
        qDebug() << "❌ Failed to open student dashboard database:" << m_dbConnection.lastError().text();
    } else {
        qDebug() << "✅ Student dashboard database opened successfully";
    }

    // Set default avatar initials
    ui->labelAvatar->setText("??");
    ui->labelFooterText->setText("Waiting for card tap...");
}

StudentDashboardPage::~StudentDashboardPage()
{
    if (m_dbConnection.isOpen()) {
        m_dbConnection.close();
    }
    if (QSqlDatabase::contains("student_dashboard_connection")) {
        QSqlDatabase::removeDatabase("student_dashboard_connection");
    }
    delete ui;
}

bool StudentDashboardPage::ensureDatabaseOpen()
{
    // Try primary database first
    if (m_db && m_db->isOpen()) {
        return true;
    }

    // Try the separate connection
    if (m_dbConnection.isOpen()) {
        return true;
    }

    // Try to reopen the separate connection
    qDebug() << "⚠️ Reopening student dashboard database connection...";

    if (QSqlDatabase::contains("student_dashboard_connection")) {
        m_dbConnection = QSqlDatabase::database("student_dashboard_connection");
    } else {
        m_dbConnection = QSqlDatabase::addDatabase("QSQLITE", "student_dashboard_connection");
    }

    QString dbPath = QDir::currentPath() + "/ku_rfid.db";
    m_dbConnection.setDatabaseName(dbPath);

    if (m_dbConnection.open()) {
        qDebug() << "✅ Student dashboard database reopened successfully";
        return true;
    } else {
        qDebug() << "❌ Failed to reopen database:" << m_dbConnection.lastError().text();
        return false;
    }
}

bool StudentDashboardPage::executeQuery(QSqlQuery &query, const QString &sql)
{
    if (!ensureDatabaseOpen()) {
        qDebug() << "❌ Database not open for query";
        return false;
    }

    query = QSqlQuery(m_dbConnection);
    if (!query.prepare(sql)) {
        qDebug() << "❌ Failed to prepare query:" << query.lastError().text();
        qDebug() << "   SQL:" << sql;
        return false;
    }
    return true;
}

bool StudentDashboardPage::bindAndExecute(QSqlQuery &query)
{
    if (!query.exec()) {
        qDebug() << "❌ Query execution failed:" << query.lastError().text();
        return false;
    }
    return true;
}

void StudentDashboardPage::refreshData()
{
    if (!ensureDatabaseOpen()) {
        ui->labelFooterText->setText("❌ Database connection failed");
        return;
    }

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

    if (!ensureDatabaseOpen()) {
        ui->labelAvatar->setText("❌");
        ui->labelStudentName->setText("Database error");
        ui->labelStudentMeta->setText("Please contact administrator");
        ui->labelCardStatus->setText("❌ DB Error");
        ui->labelFooterText->setText("❌ Database connection failed");
        return;
    }

    // Get student by RFID using the separate connection
    QSqlQuery query(m_dbConnection);
    query.prepare("SELECT id, name, roll_no, rfid_card_id, balance, class_section, registration_date "
                  "FROM students WHERE rfid_card_id = ?");
    query.addBindValue(cardId);

    if (!query.exec()) {
        qDebug() << "❌ Failed to query student:" << query.lastError().text();
        ui->labelAvatar->setText("❌");
        ui->labelStudentName->setText("Database error");
        ui->labelStudentMeta->setText("Query failed");
        ui->labelCardStatus->setText("❌ Error");
        ui->labelFooterText->setText("❌ Database query failed");
        return;
    }

    if (!query.next()) {
        qDebug() << "❌ Student not found for RFID:" << cardId;
        ui->labelAvatar->setText("❌");
        ui->labelStudentName->setText("Card not recognized");
        ui->labelStudentMeta->setText("Please contact the administrator");
        ui->labelCardStatus->setText("❌ Invalid card");
        ui->labelFooterText->setText("❌ Card ID " + cardId + " not recognized");
        return;
    }

    Student student;
    student.id = query.value("id").toInt();
    student.name = query.value("name").toString();
    student.rollNo = query.value("roll_no").toString();
    student.rfidCardId = query.value("rfid_card_id").toString();
    student.balance = query.value("balance").toInt();
    student.classSection = query.value("class_section").toString();
    student.registrationDate = query.value("registration_date").toString();

    qDebug() << "✅ Student found:" << student.name << "(ID:" << student.id << ")";

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
    QSqlQuery totalQuery(m_dbConnection);
    totalQuery.prepare("SELECT COUNT(DISTINCT date) FROM attendance");
    if (totalQuery.exec() && totalQuery.next()) {
        totalClasses = totalQuery.value(0).toInt();
    } else {
        qDebug() << "❌ Failed to get total classes:" << totalQuery.lastError().text();
    }

    // Get attended classes for this student
    int attended = 0;
    QSqlQuery attQuery(m_dbConnection);
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
    QSqlQuery todayQuery(m_dbConnection);
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

    int balance = 0;
    QSqlQuery query(m_dbConnection);
    query.prepare("SELECT balance FROM students WHERE id = ?");
    query.addBindValue(studentId);
    if (query.exec() && query.next()) {
        balance = query.value(0).toInt();
    }

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

    QSqlQuery query(m_dbConnection);
    query.prepare("SELECT item, amount, date, time FROM cafeteria_transactions "
                  "WHERE student_id = ? ORDER BY date DESC, time DESC LIMIT 3");
    query.addBindValue(studentId);

    if (!query.exec()) {
        qDebug() << "❌ Failed to get transactions:" << query.lastError().text();
        return;
    }

    int row = 0;
    while (query.next() && row < 3) {
        QString item = query.value("item").toString();
        int amount = query.value("amount").toInt();
        QString date = query.value("date").toString();
        QString time = query.value("time").toString();

        if (row == 0) {
            ui->transName1->setText(item);
            ui->transDate1->setText(date + ", " + time);
            ui->transAmt1->setText("- Rs. " + QString::number(amount));
            ui->transAmt1->setStyleSheet("color:#E8E8F0; font-size:14px; font-weight:600; background:transparent; border:none;");
        } else if (row == 1) {
            ui->transName2->setText(item);
            ui->transDate2->setText(date + ", " + time);
            ui->transAmt2->setText("- Rs. " + QString::number(amount));
            ui->transAmt2->setStyleSheet("color:#E8E8F0; font-size:14px; font-weight:600; background:transparent; border:none;");
        } else if (row == 2) {
            ui->transName3->setText(item);
            ui->transDate3->setText(date + ", " + time);
            ui->transAmt3->setText("- Rs. " + QString::number(amount));
            ui->transAmt3->setStyleSheet("color:#E8E8F0; font-size:14px; font-weight:600; background:transparent; border:none;");
        }
        row++;
    }
}

void StudentDashboardPage::updateTodayMeals(int studentId)
{
    if (!ensureDatabaseOpen()) return;

    QDate today = QDate::currentDate();
    QString todayStr = today.toString("yyyy-MM-dd");

    // Get today's transactions
    QList<CafeteriaTransaction> todayTransactions;
    QSqlQuery query(m_dbConnection);
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