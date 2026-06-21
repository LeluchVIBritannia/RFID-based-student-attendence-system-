#include "RegisterCardPage.h"
#include "ui_RegisterCardPage.h"

#include <QDateTime>
#include <QMessageBox>
#include <QLabel>
#include <QPushButton>
#include <QDebug>

RegisterCardPage::RegisterCardPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::RegisterCardPage), m_db(DatabaseManager::instance()), m_currentStudentId(-1)
{
    ui->setupUi(this);
    wireForm();
}

RegisterCardPage::~RegisterCardPage()
{
    delete ui;
}

void RegisterCardPage::refreshData()
{
    clearForm();
    m_currentStudentId = -1;
}

void RegisterCardPage::wireForm()
{
    // Connect buttons - using the slot names from the header
    connect(ui->pushButtonScanCard, &QPushButton::clicked, this, &RegisterCardPage::onScanCard);
    connect(ui->pushButtonRegisterClear, &QPushButton::clicked, this, &RegisterCardPage::onClearForm);
    connect(ui->pushButtonRegisterSave, &QPushButton::clicked, this, &RegisterCardPage::onSaveStudent);

    // Connect Delete button
    QPushButton *deleteButton = findChild<QPushButton*>("pushButtonDelete");
    if (deleteButton) {
        connect(deleteButton, &QPushButton::clicked, this, &RegisterCardPage::onDeleteStudent);
    } else {
        deleteButton = findChild<QPushButton*>("btnDelete");
        if (deleteButton) {
            connect(deleteButton, &QPushButton::clicked, this, &RegisterCardPage::onDeleteStudent);
        }
    }
}

void RegisterCardPage::onScanCard()
{
    ui->lineEditRfidId->setText("RFID-" + QString::number(QDateTime::currentMSecsSinceEpoch() % 100000000));
    ui->labelRegStatus->setStyleSheet("color:#3498DB; font-size:12px; background:transparent; border:none;");
    ui->labelRegStatus->setText("Card scanned. Fill in the student details and click Save.");
}

void RegisterCardPage::onClearForm()
{
    clearForm();
    m_currentStudentId = -1;
    ui->labelRegStatus->setText("");
}

void RegisterCardPage::onSaveStudent()
{
    saveStudent();
}

void RegisterCardPage::onDeleteStudent()
{
    if (m_currentStudentId == -1) {
        // Try to get student from form
        Student student = getStudentFromForm();
        if (student.id == -1) {
            QMessageBox::warning(this, "No Student Selected",
                                 "Please load a student first by entering Roll No. or scanning RFID.");
            return;
        }
        m_currentStudentId = student.id;
    }

    Student student = m_db->getStudentById(m_currentStudentId);
    if (student.id == -1) {
        QMessageBox::warning(this, "Error", "Student not found in database.");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Confirm Delete",
        QString("Are you sure you want to delete student '%1' (Roll No: %2)?\n\n"
                "This action CANNOT be undone!")
            .arg(student.name)
            .arg(student.rollNo),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::No) {
        return;
    }

    bool success = m_db->deleteStudent(student.id);

    if (success) {
        QMessageBox::information(this, "Success",
                                 QString("Student '%1' deleted successfully.").arg(student.name));
        clearForm();
        m_currentStudentId = -1;
        ui->labelRegStatus->setStyleSheet("color:#2ECC71; font-size:12px; background:transparent; border:none;");
        ui->labelRegStatus->setText("Student deleted successfully!");
        emit studentAdded();
    } else {
        QMessageBox::critical(this, "Error", "Failed to delete student.");
    }
}

void RegisterCardPage::clearForm()
{
    ui->lineEditRfidId->clear();
    ui->lineEditRegName->clear();
    ui->lineEditRegRoll->clear();
    ui->lineEditRegSection->clear();
    ui->lineEditRegBalance->clear();
    ui->labelRegStatus->setText("");
    m_currentStudentId = -1;
}

bool RegisterCardPage::validateForm()
{
    if (ui->lineEditRfidId->text().trimmed().isEmpty() ||
        ui->lineEditRegName->text().trimmed().isEmpty() ||
        ui->lineEditRegRoll->text().trimmed().isEmpty()) {

        ui->labelRegStatus->setStyleSheet("color:#E74C3C; font-size:12px; background:transparent; border:none;");
        ui->labelRegStatus->setText("Please fill in RFID Card ID, Name, and Roll No.");
        return false;
    }
    return true;
}

bool RegisterCardPage::saveStudent()
{
    if (!m_db) {
        ui->labelRegStatus->setStyleSheet("color:#E74C3C; font-size:12px; background:transparent; border:none;");
        ui->labelRegStatus->setText("Database not available!");
        return false;
    }

    if (!validateForm()) return false;

    QString rfid = ui->lineEditRfidId->text().trimmed();
    QString name = ui->lineEditRegName->text().trimmed();
    QString roll = ui->lineEditRegRoll->text().trimmed();
    QString section = ui->lineEditRegSection->text().trimmed();
    int balance = ui->lineEditRegBalance->text().toInt();

    bool success = m_db->addStudent(name, roll, rfid, balance, section);

    if (success) {
        ui->labelRegStatus->setStyleSheet("color:#2ECC71; font-size:12px; background:transparent; border:none;");
        ui->labelRegStatus->setText("Student registered successfully!");
        clearForm();
        emit studentAdded();
        return true;
    } else {
        ui->labelRegStatus->setStyleSheet("color:#E74C3C; font-size:12px; background:transparent; border:none;");
        ui->labelRegStatus->setText("Failed to register. RFID or Roll No. may already exist.");
        return false;
    }
}

Student RegisterCardPage::getStudentFromForm()
{
    QString rollNo = ui->lineEditRegRoll->text().trimmed();
    if (rollNo.isEmpty()) {
        return Student();
    }
    return m_db->getStudentByRoll(rollNo);
}