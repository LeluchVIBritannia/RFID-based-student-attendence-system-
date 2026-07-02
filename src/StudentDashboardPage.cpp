#include "StudentDashboardPage.h"
#include "ui_StudentDashboardPage.h"

StudentDashboardPage::StudentDashboardPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::StudentDashboardPage)
{
    ui->setupUi(this);
    populateDemoData();
}

StudentDashboardPage::~StudentDashboardPage()
{
    delete ui;
}

// ── Demo data matching the design preview ───────────────────────────────────
void StudentDashboardPage::populateDemoData()
{
    ui->labelAvatar->setText("KS");
    ui->labelStudentName->setText("Krimud Sainju");
    ui->labelStudentMeta->setText("Roll No. 08  ·  CSE-I/II  ·  Kathmandu University");
    ui->labelCardStatus->setText("✓  Card active");

    ui->sc1Value->setText("Marked");
    ui->sc1Sub->setText("09:02 AM");

    ui->sc2Value->setText("87%");
    ui->sc2Sub->setText("27 of 31 classes");

    ui->sc3Value->setText("Rs. 1,200");
    ui->sc3Sub->setText("Last topped up Jun 12");

    ui->mealItem1->setText("Lunch set");
    ui->mealTime1->setText("12:05 PM");
    ui->mealItem2->setText("Tea + snacks");
    ui->mealTime2->setText("10:30 AM");
    ui->labelMealFooter->setText("2 purchases · Rs. 245 spent today");

    ui->transName1->setText("Lunch set");
    ui->transDate1->setText("Today, 12:05 PM");
    ui->transAmt1->setText("- Rs. 180");

    ui->transName2->setText("Tea + snacks");
    ui->transDate2->setText("Today, 10:30 AM");
    ui->transAmt2->setText("- Rs. 65");

    ui->transName3->setText("Balance top-up");
    ui->transDate3->setText("Jun 12, 4:20 PM");
    ui->transAmt3->setText("+ Rs. 1,000");

    ui->labelFooterText->setText("Card ID A1B2C3D4 · Tap your card again anytime to refresh this view");
}

// ── Called whenever a real RFID card is scanned ──────────────────────────────
// TODO: Replace with a MySQL lookup using cardId:
//   SELECT name, roll, balance, attendance_pct FROM students WHERE rfid_id = ?
//   SELECT * FROM transactions WHERE student_id = ? ORDER BY time DESC LIMIT 3
void StudentDashboardPage::loadStudentByCardId(const QString &cardId)
{
    ui->labelFooterText->setText(QString("Card ID %1 · Tap your card again anytime to refresh this view").arg(cardId));
    // For now this just keeps the demo data on screen.
    // Once a database layer is added, fetch and apply real values here.
}
