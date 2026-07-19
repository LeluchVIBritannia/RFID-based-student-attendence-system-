#include "StudentDashboardPage.h"
#include "ui_StudentDashboardPage.h"

#include <QDebug>
#include <QDateTime>
#include <QSqlQuery>
#include <QMessageBox>
#include <QInputDialog>
#include <QTableWidgetItem>

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

void StudentDashboardPage::refreshData()
{
    if (!ensureDatabaseOpen()) {
        ui->labelFooterText->setText("Database connection failed");
        return;
    }

    if (m_currentStudent.id != -1) {
        updateUI(m_currentStudent);
    } else {
        ui->labelAvatar->setText("??");
        ui->labelStudentName->setText("Tap your RFID card");
        ui->labelStudentMeta->setText("to view your dashboard");
        ui->labelCardStatus->setText("Waiting...");

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
    qDebug() << "Loading student dashboard for RFID:" << cardId;

    if (!ensureDatabaseOpen()) {
        ui->labelAvatar->setText("XXX");
        ui->labelStudentName->setText("Database error");
        ui->labelStudentMeta->setText("Please contact administrator");
        ui->labelCardStatus->setText("DB Error");
        ui->labelFooterText->setText("Database connection failed");
        return;
    }

    Student student = m_db->getStudentByRFID(cardId);
    if (student.id == -1) {
        qDebug() << "Student not found for RFID:" << cardId;
        ui->labelAvatar->setText("XXX");
        ui->labelStudentName->setText("Card not recognized");
        ui->labelStudentMeta->setText("Please contact the administrator");
        ui->labelCardStatus->setText("Invalid card");
        ui->labelFooterText->setText("Card ID " + cardId + " not recognized");
        return;
    }

    qDebug() << "Student found:" << student.name << "(ID:" << student.id << ")";
    m_currentStudent = student;
    m_currentBalance = m_db->getStudentBalance(student.id);
    updateUI(student);
}


void StudentDashboardPage::updateUI(const Student &student)
{
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

    updateAttendanceStats(student.id);
    updateBalance(student.id);

    ui->labelFooterText->setText(QString("Card ID %1 · Tap your card again anytime to refresh this view")
                                     .arg(student.rfidCardId));

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
        ui->sc1Value->setText("Marked");
        ui->sc1Value->setStyleSheet("color:#2ECC71; font-size:24px; font-weight:700; background:transparent; border:none;");
        ui->sc1Sub->setText(todayTime);
    } else {
        ui->sc1Value->setText("Not marked");
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

void StudentDashboardPage::updateRecentTransactions(int studentId)
{
    Q_UNUSED(studentId);
}

void StudentDashboardPage::updateTodayMeals(int studentId)
{
    Q_UNUSED(studentId);
}

// ----------------------------------------------------------------------
// Food page table population (purchase HISTORY, not the menu)
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

    ui->tableTodayMeals->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

// ----------------------------------------------------------------------
// Slot implementations
// ----------------------------------------------------------------------

void StudentDashboardPage::onBuyFoodClicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    updateFoodPage();
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

    if (!ensureDatabaseOpen()) {
        QMessageBox::critical(this, "Error", "Database not available");
        return;
    }

    // Pull the live menu from the database (same table CafeteriaPage reads from)
    QList<MenuItem> menuItems = m_db->getAllMenuItems();
    if (menuItems.isEmpty()) {
        QMessageBox::warning(this, "No Items", "No menu items configured in the database.");
        return;
    }

    QStringList itemLabels;
    for (const MenuItem &mi : menuItems) {
        itemLabels << QString("%1 (Rs. %2)").arg(mi.name).arg(mi.price);
    }

    bool ok;
    QString selected = QInputDialog::getItem(this, "Buy Food & Drinks",
                                             QString("Balance: Rs. %1\n\nSelect an item to purchase:")
                                                 .arg(m_currentBalance),
                                             itemLabels, 0, false, &ok);

    if (!ok || selected.isEmpty()) {
        return; // user cancelled
    }

    int idx = itemLabels.indexOf(selected);
    if (idx < 0 || idx >= menuItems.size()) {
        return; // shouldn't happen, but guard anyway
    }
    const MenuItem &chosen = menuItems[idx];

    if (m_currentBalance < chosen.price) {
        QMessageBox::warning(this, "Insufficient Balance",
                             QString("You need Rs. %1 more to buy %2.")
                                 .arg(chosen.price - m_currentBalance)
                                 .arg(chosen.name));
        return;
    }

    bool success = m_db->recordCafeteriaTransaction(m_currentStudent.id, chosen.name, chosen.price);

    if (!success) {
        QMessageBox::critical(this, "Error", "Purchase failed. Please try again.");
        return;
    }

    // Re-read from the database rather than mutating local state by hand
    updateBalance(m_currentStudent.id);
    updateFoodPage();

    QMessageBox::information(this, "Purchase Successful",
                             QString("You bought %1 for Rs. %2.\nRemaining balance: Rs. %3")
                                 .arg(chosen.name)
                                 .arg(chosen.price)
                                 .arg(m_currentBalance));

    ui->stackedWidget->setCurrentIndex(0);
}