#include "ScanTerminalPage.h"
#include "ui_ScanTerminalPage.h"
#include "MainWindow.h"
#include "SerialManager.h"  // ADD THIS

#include <QLabel>
#include <QDateTime>
#include <QDebug>

ScanTerminalPage::ScanTerminalPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ScanTerminalPage)
    , m_db(DatabaseManager::instance())
    , m_serialManager(new SerialManager(this))  // CREATE SerialManager
{
    ui->setupUi(this);
    wireButtons();
    refreshData();

    // CONNECT SerialManager to this page
    connect(m_serialManager, &SerialManager::cardScanned,
            this, &ScanTerminalPage::onSerialCardScanned);
}

ScanTerminalPage::~ScanTerminalPage()
{
    delete ui;
}

void ScanTerminalPage::refreshData()
{
    setState("📡", "Waiting for Card...", "Tap an RFID card on the reader to begin", "#E8E8F0", "#3A3A5A");
}

// THIS FUNCTION RECEIVES THE UID FROM SERIAL MANAGER
void ScanTerminalPage::onSerialCardScanned(const QString &uid)
{
    qDebug() << "📇 Card scanned from serial:" << uid;
    onRFIDScanned(uid, "attendance");
}

void ScanTerminalPage::onRFIDScanned(const QString &rfidCardId, const QString &mode)
{
    if (!m_db) {
        setState("❌", "System Error", "Database not available", "#E74C3C", "#E74C3C");
        return;
    }

    Student student = m_db->getStudentByRFID(rfidCardId);

    if (student.id == -1) {
        setState("❌", "Invalid Card", "Card not recognized", "#E74C3C", "#E74C3C");
        return;
    }

    if (mode == "attendance") {
        bool success = m_db->recordAttendance(student.id, "Morning");
        if (success) {
            QString time = QDateTime::currentDateTime().toString("hh:mm AP");
            setState("✅", "Attendance Marked",
                     QString("%1 (Roll %2) — %3").arg(student.name).arg(student.rollNo).arg(time),
                     "#2ECC71", "#2ECC71");
            navigateToStudentDashboard(rfidCardId);
        } else {
            setState("⚠️", "Already Recorded",
                     "Attendance already marked", "#F39C12", "#F39C12");
            navigateToStudentDashboard(rfidCardId);
        }
    }
}

void ScanTerminalPage::navigateToStudentDashboard(const QString &rfidCardId)
{
    QWidget *parent = parentWidget();
    while (parent && !parent->inherits("MainWindow")) {
        parent = parent->parentWidget();
    }

    if (parent) {
        MainWindow *mainWin = qobject_cast<MainWindow*>(parent);
        if (mainWin) {
            qDebug() << "📱 Navigating to Student Dashboard for RFID:" << rfidCardId;
            mainWin->showStudentDashboard(rfidCardId);
        }
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
    // Demo buttons for testing without hardware
    connect(ui->pushButtonDemoMarked, &QPushButton::clicked, this, [this]() {
        QList<Student> students = m_db->getAllStudents();
        if (!students.isEmpty()) {
            onRFIDScanned(students[0].rfidCardId, "attendance");
        }
    });

    connect(ui->pushButtonDemoAlready, &QPushButton::clicked, this, [this]() {
        QList<Student> students = m_db->getAllStudents();
        if (!students.isEmpty()) {
            onRFIDScanned(students[0].rfidCardId, "attendance");
        }
    });

    connect(ui->pushButtonDemoInvalid, &QPushButton::clicked, this, [this]() {
        onRFIDScanned("INVALID_CARD", "attendance");
    });

    connect(ui->pushButtonDemoCafeteria, &QPushButton::clicked, this, [this]() {
        QList<Student> students = m_db->getAllStudents();
        if (!students.isEmpty()) {
            onRFIDScanned(students[0].rfidCardId, "cafeteria");
        }
    });

    connect(ui->pushButtonDemoReset, &QPushButton::clicked, this, [this]() {
        refreshData();
    });
}