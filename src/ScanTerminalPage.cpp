#include "ScanTerminalPage.h"
#include "ui_ScanTerminalPage.h"
#include "MainWindow.h"

#include <QLabel>
#include <QDateTime>
#include <QDebug>

ScanTerminalPage::ScanTerminalPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::ScanTerminalPage), m_db(DatabaseManager::instance())
{
    ui->setupUi(this);
    wireButtons();
    refreshData();
}

ScanTerminalPage::~ScanTerminalPage()
{
    delete ui;
}

void ScanTerminalPage::refreshData()
{
    setState("......", "Waiting for Card...", "Tap an RFID card on the reader to begin", "#E8E8F0", "#3A3A5A");
}

void ScanTerminalPage::onRFIDScanned(const QString &rfidCardId, const QString &mode)
{
    if (!m_db) {
        setState("XXX", "System Error", "Database not available", "#E74C3C", "#E74C3C");
        return;
    }

    Student student = m_db->getStudentByRFID(rfidCardId);

    if (student.id == -1) {
        setState("XXX", "Invalid Card", "Card not recognized — please contact the administrator", "#E74C3C", "#E74C3C");
        return;
    }

    if (mode == "attendance") {
        bool success = m_db->recordAttendance(student.id, "Morning");
        if (success) {
            QString time = QDateTime::currentDateTime().toString("hh:mm AP");
            setState(":)", "Attendance Marked",
                     QString("%1 (Roll %2) — %3").arg(student.name).arg(student.rollNo).arg(time),
                     "#2ECC71", "#2ECC71");

            // AFTER MARKING ATTENDANCE, NAVIGATE TO STUDENT DASHBOARD
            navigateToStudentDashboard(rfidCardId);
        } else {
            setState(":|", "Already Recorded",
                     "Attendance for this session was already marked", "#F39C12", "#F39C12");

            // STILL NAVIGATE TO STUDENT DASHBOARD TO SHOW THEIR STATUS
            navigateToStudentDashboard(rfidCardId);
        }
    } else if (mode == "cafeteria") {
        int balance = m_db->getStudentBalance(student.id);
        setState(":)", "Student Found",
                 QString("%1 (Roll %2) — Balance: Rs. %3").arg(student.name).arg(student.rollNo).arg(balance),
                 "#3498DB", "#3498DB");

        // NAVIGATE TO STUDENT DASHBOARD FOR CAFETERIA VIEW
        navigateToStudentDashboard(rfidCardId);
    } else {
        // Default: just show student dashboard
        navigateToStudentDashboard(rfidCardId);
    }
}


void ScanTerminalPage::navigateToStudentDashboard(const QString &rfidCardId)
{
    // Find the MainWindow instance
    QWidget *parent = parentWidget();
    while (parent && !parent->inherits("MainWindow")) {
        parent = parent->parentWidget();
    }

    if (parent) {
        MainWindow *mainWin = qobject_cast<MainWindow*>(parent);
        if (mainWin) {
            qDebug() << "Navigating to Student Dashboard for RFID:" << rfidCardId;
            mainWin->showStudentDashboard(rfidCardId);
        } else {
            qDebug() << "Failed to cast to MainWindow";
        }
    } else {
        qDebug() << " MainWindow not found in parent chain";
    }
}

void ScanTerminalPage::setState(const QString &icon, const QString &msg, const QString &detail,
                                const QString &color, const QString &borderColor)
{
    ui->labelStatusIcon->setText(icon);
    ui->labelStatusMessage->setText(msg);
    ui->labelStatusMessage->setStyleSheet(
        QString("color:%1; font-size:28px; font-weight:800; background:transparent; border:none;").arg(color));
    ui->labelStatusDetail->setText(detail);
    ui->scanStatusFrame->setStyleSheet(
        QString("QFrame#scanStatusFrame{background-color:#24243A;border:2px solid %1;border-radius:16px;}").arg(borderColor));
}

void ScanTerminalPage::wireButtons()
{
    connect(ui->pushButtonDemoMarked, &QPushButton::clicked, this, [this]() {
        onRFIDScanned("A1B2C3D4", "attendance");
    });

    connect(ui->pushButtonDemoAlready, &QPushButton::clicked, this, [this]() {
        onRFIDScanned("A1B2C3D4", "attendance");
    });

    connect(ui->pushButtonDemoInvalid, &QPushButton::clicked, this, [this]() {
        onRFIDScanned("INVALID_CARD", "attendance");
    });

    connect(ui->pushButtonDemoCafeteria, &QPushButton::clicked, this, [this]() {
        onRFIDScanned("A1B2C3D4", "cafeteria");
    });

    connect(ui->pushButtonDemoReset, &QPushButton::clicked, this, [this]() {
        refreshData();
    });
}