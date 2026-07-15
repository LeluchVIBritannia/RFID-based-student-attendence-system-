#include "StudentDashboardPage.h"
#include "ui_StudentDashboardPage.h"

#include <QDebug>
#include <QDateTime>
#include <QSqlQuery>
#include <QMessageBox>
#include <QTableWidgetItem>   // for table items

StudentDashboardPage::StudentDashboardPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StudentDashboardPage)
    , m_db(DatabaseManager::instance())
    , m_currentBalance(0)
{
    ui->setupUi(this);

    // Show info view by default (stacked widget index 0)
    ui->stackedWidget->setCurrentIndex(0);

    // Set default avatar initials
    ui->labelAvatar->setText("??");
    ui->labelFooterText->setText("Waiting for card tap...");

    // Ensure database is open
    if (m_db && !m_db->isOpen()) {
        qDebug() << "⚠️ Database not open, reinitializing...";
        m_db->initDatabase();
    }

    // Connect buttons
    connect(ui->btnBuyFood, &QPushButton::clicked, this, &StudentDashboardPage::onBuyFoodClicked);
    connect(ui->btnBackToInfo, &QPushButton::clicked, this, &StudentDashboardPage::onBackToInfoClicked);
    connect(ui->btnPurchase, &QPushButton::clicked, this, &StudentDashboardPage::onPurchaseClicked);
}

StudentDashboardPage::~StudentDashboardPage()
{
    delete ui;
}

// ----------------------------------------------------------------------
// Database helper
// ----------------------------------------------------------------------
bool StudentDashboardPage::ensureDatabaseOpen()
{
    if (!m_db) return false;
    if (!m_db->isOpen()) {
        m_db->initDatabase();
        return m_db->isOpen();
    }
    return true;
}

// ----------------------------------------------------------------------
// Public slots / entry points
// ----------------------------------------------------------------------
void StudentDashboardPage::refreshData()
{
    if (!ensureDatabaseOpen()) {
        ui->labelFooterText->setText("❌ Database connection failed");
        return;
    }

    // If we have a student loaded, reload their data; otherwise show waiting state.
    if (m_currentStudent.id != -1) {
        updateUI(m_currentStudent);
    } else {
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

        ui->labelFooterText->setText("Tap your RFID card to view your dashboard");
    }
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
    m_currentStudent = student;
    m_currentBalance = m_db->getStudentBalance(student.id);
    updateUI(student);
}

// ----------------------------------------------------------------------
// UI updates
// ----------------------------------------------------------------------
void StudentDashboardPage::updateUI(const Student &student)
{
    // Avatar initials
    QStringList nameParts = student.name.split(" ");
    QString initials;
    if (nameParts.size() >= 2)
        initials = nameParts[0].left(1).toUpper() + nameParts[nameParts.size()-1].left(1).toUpper();
    else if (nameParts.size() == 1)
        initials = nameParts[0].left(2).toUpper();
    else
        initials = "??";
    ui->labelAvatar->setText(initials);

    ui->labelStudentName->setText(student.name);
    ui->labelStudentMeta->setText(QString("Roll No. %1  ·  %2  ·  Kathmandu University")
                                      .arg(student.rollNo)
                                      .arg(student.classSection));
    ui->labelCardStatus->setText("✓  Card active");
    ui->labelCardStatus->setStyleSheet("background-color:#1E3D2B; color:#2ECC71; border-radius:15px; padding:0 16px; font-size:13px; font-weight:600;");

    // Update the three stat cards
    updateAttendanceStats(student.id);
    updateBalance(student.id);

    // Footer
    ui->labelFooterText->setText(QString("Card ID %1 · Tap your card again anytime to refresh this view")
                                     .arg(student.rfidCardId));

    // Ensure we are on the info view
    ui->stackedWidget->setCurrentIndex(0);
}

// ----------------------------------------------------------------------
// Stats update functions
// ----------------------------------------------------------------------
void StudentDashboardPage::updateAttendanceStats(int studentId)
{
    if (!ensureDatabaseOpen()) return;

    int totalClasses = 0;
    QSqlQuery totalQuery(m_db->getDatabase());
    totalQuery.prepare("SELECT COUNT(DISTINCT date) FROM attendance");
    if (totalQuery.exec() && totalQuery.next())
        totalClasses = totalQuery.value(0).toInt();

    int attended = 0;
    QSqlQuery attQuery(m_db->getDatabase());
    attQuery.prepare("SELECT COUNT(*) FROM attendance WHERE student_id = ?");
    attQuery.addBindValue(studentId);
    if (attQuery.exec() && attQuery.next())
        attended = attQuery.value(0).toInt();

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

    int percentage = (totalClasses > 0) ? (attended * 100 / totalClasses) : 0;

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

    if (percentage >= 75)
        ui->sc2Value->setStyleSheet("color:#2ECC71; font-size:24px; font-weight:700; background:transparent; border:none;");
    else if (percentage >= 50)
        ui->sc2Value->setStyleSheet("color:#F39C12; font-size:24px; font-weight:700; background:transparent; border:none;");
    else
        ui->sc2Value->setStyleSheet("color:#E74C3C; font-size:24px; font-weight:700; background:transparent; border:none;");
}

void StudentDashboardPage::updateBalance(int studentId)
{
    if (!ensureDatabaseOpen()) return;

    int balance = m_db->getStudentBalance(studentId);
    m_currentBalance = balance;
    ui->sc3Value->setText(QString("Rs. %1").arg(balance));
    ui->sc3Sub->setText(QString("Balance updated %1").arg(QDate::currentDate().toString("MMM dd")));

    if (balance > 500)
        ui->sc3Value->setStyleSheet("color:#2ECC71; font-size:24px; font-weight:700; background:transparent; border:none;");
    else if (balance > 100)
        ui->sc3Value->setStyleSheet("color:#F39C12; font-size:24px; font-weight:700; background:transparent; border:none;");
    else
        ui->sc3Value->setStyleSheet("color:#E74C3C; font-size:24px; font-weight:700; background:transparent; border:none;");
}

// Keep these two functions (they are not called in updateUI anymore)
// but they might be reused later.
void StudentDashboardPage::updateRecentTransactions(int studentId)
{
    // Not used in current UI, but kept for potential later use.
    Q_UNUSED(studentId);
}

void StudentDashboardPage::updateTodayMeals(int studentId)
{
    // Not used in current UI, but kept for potential later use.
    Q_UNUSED(studentId);
}

// ----------------------------------------------------------------------
// Food page table population
// ----------------------------------------------------------------------
void StudentDashboardPage::updateFoodPage()
{
    if (!ensureDatabaseOpen() || m_currentStudent.id == -1) {
        ui->tableTodayMeals->setRowCount(0);
        ui->labelFoodMessage->setText("No student loaded");
        return;
    }

    QDate today = QDate::currentDate();
    QString todayStr = today.toString("yyyy-MM-dd");

    QList<CafeteriaTransaction> todayTransactions;
    QSqlQuery query(m_db->getDatabase());
    query.prepare("SELECT item, amount, time FROM cafeteria_transactions "
                  "WHERE student_id = ? AND date = ? ORDER BY time DESC");
    query.addBindValue(m_currentStudent.id);
    query.addBindValue(todayStr);
    if (query.exec()) {
        while (query.next()) {
            CafeteriaTransaction trans;
            trans.item = query.value("item").toString();
            trans.amount = query.value("amount").toInt();
            trans.time = query.value("time").toString();
            todayTransactions.append(trans);
        }
    }

    int rowCount = todayTransactions.size();
    ui->tableTodayMeals->setRowCount(rowCount);

    if (rowCount == 0) {
        ui->labelFoodMessage->setText("No purchases today");
    } else {
        ui->labelFoodMessage->setText(QString("%1 purchase(s) today").arg(rowCount));
        for (int i = 0; i < rowCount; ++i) {
            ui->tableTodayMeals->setItem(i, 0, new QTableWidgetItem(todayTransactions[i].item));
            ui->tableTodayMeals->setItem(i, 1, new QTableWidgetItem(todayTransactions[i].time));
            ui->tableTodayMeals->setItem(i, 2, new QTableWidgetItem("Rs. " + QString::number(todayTransactions[i].amount)));
        }
    }

    // Make columns stretch to fill the table width
    ui->tableTodayMeals->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

// ----------------------------------------------------------------------
// Slot implementations
// ----------------------------------------------------------------------
void StudentDashboardPage::onBuyFoodClicked()
{
    // Switch to food purchase page
    ui->stackedWidget->setCurrentIndex(1);
    // Populate the table with today's meals
    updateFoodPage();
    // Refresh balance (just in case)
    if (m_currentStudent.id != -1)
        updateBalance(m_currentStudent.id);
}

void StudentDashboardPage::onBackToInfoClicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void StudentDashboardPage::onPurchaseClicked()
{
    if (m_currentStudent.id == -1) {
        QMessageBox::warning(this, "Error", "No student loaded");
        return;
    }

    int price = 100; // fixed price for demo

    if (m_currentBalance < price) {
        QMessageBox::warning(this, "Insufficient Balance",
                             QString("You need Rs. %1 more to buy this item.")
                                 .arg(price - m_currentBalance));
        return;
    }

    // ----- MOCK PURCHASE (no database write yet) -----
    // In a real app you would call:
    //   m_db->deductBalance(m_currentStudent.id, price);
    //   m_db->recordCafeteriaTransaction(m_currentStudent.id, "Meal", price, "Purchase");
    // For now, we just update the local balance and the table.

    m_currentBalance -= price;
    // Update the balance label manually
    ui->sc3Value->setText(QString("Rs. %1").arg(m_currentBalance));

    // Add a mock row to the table (for demonstration)
    int row = ui->tableTodayMeals->rowCount();
    ui->tableTodayMeals->insertRow(row);
    ui->tableTodayMeals->setItem(row, 0, new QTableWidgetItem("Meal (mock)"));
    ui->tableTodayMeals->setItem(row, 1, new QTableWidgetItem(QTime::currentTime().toString("hh:mm AP")));
    ui->tableTodayMeals->setItem(row, 2, new QTableWidgetItem("Rs. " + QString::number(price)));
    ui->labelFoodMessage->setText(QString("%1 purchase(s) today").arg(row+1));

    QMessageBox::information(this, "Purchase Successful",
                             QString("You bought a meal for Rs. %1.\nRemaining balance: Rs. %2")
                                 .arg(price)
                                 .arg(m_currentBalance));

    // Go back to info view
    ui->stackedWidget->setCurrentIndex(0);
}