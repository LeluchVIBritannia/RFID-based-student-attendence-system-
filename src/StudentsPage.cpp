#include "StudentsPage.h"
#include "ui_StudentsPage.h"
#include "DashboardPage.h"

#include <QTableWidgetItem>
#include <QColor>
#include <QFont>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlQuery>
#include <QDebug>

static QTableWidgetItem* colorItem(const QString &text, const QString &hex)
{
    auto *item = new QTableWidgetItem(text);
    item->setForeground(QColor(hex));
    return item;
}

StudentsPage::StudentsPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::StudentsPage), m_db(DatabaseManager::instance())
{
    ui->setupUi(this);
    connectButtons();
    refreshData();
}

StudentsPage::~StudentsPage()
{
    delete ui;
}

void StudentsPage::connectButtons()
{
    QPushButton *addButton = ui->pushButtonAddStudent;
    if (addButton) {
        connect(addButton, &QPushButton::clicked, this, [this]() {
            QWidget *parent = parentWidget();
            while (parent && !parent->inherits("DashboardPage")) {
                parent = parent->parentWidget();
            }
            if (parent) {
                DashboardPage *dashboard = qobject_cast<DashboardPage*>(parent);
                if (dashboard) dashboard->navigateToRegisterPage();
            }
        });
    }
}

void StudentsPage::onDeleteStudentClicked()
{
    Student student = getSelectedStudent();

    if (student.id == -1) {
        QMessageBox::warning(this, "No Selection", "Please select a student to delete.");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Delete",
        QString("Delete student '%1' (Roll: %2)?").arg(student.name).arg(student.rollNo),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::No) return;

    if (m_db->deleteStudent(student.id)) {
        QMessageBox::information(this, "Success", "Student deleted!");
        refreshData();
    } else {
        QMessageBox::critical(this, "Error", "Failed to delete.");
    }
}

void StudentsPage::refreshData()
{
    if (!m_db) return;
    qDebug() << "Refreshing student table...";
    updateStudentTable();
}

void StudentsPage::onStudentAdded()
{
    refreshData();
}

void StudentsPage::updateStudentTable()
{
    if (!m_db) return;

    QTableWidget *t = ui->tableStudents;
    QList<Student> students = m_db->getAllStudents();

    t->setRowCount(0);

    if (students.isEmpty()) {
        t->setRowCount(1);
        QTableWidgetItem *msgItem = new QTableWidgetItem("No students registered");
        msgItem->setTextAlignment(Qt::AlignCenter);
        t->setItem(0, 0, msgItem);
        t->setSpan(0, 0, 1, 5);
        return;
    }

    // Update column headers to show Registration Date instead of Attendance %
    t->setColumnCount(5);
    t->setHorizontalHeaderLabels({"Name", "Roll No.", "RFID Card ID", "Balance (NPR)", "Registration Date"});
    t->horizontalHeader()->setStretchLastSection(true);

    t->setRowCount(students.size());
    QFont mono("Courier New");

    for (int r = 0; r < students.size(); ++r) {
        const Student &student = students[r];

        t->setItem(r, 0, new QTableWidgetItem(student.name));
        t->setItem(r, 1, new QTableWidgetItem(student.rollNo));

        auto *rfidItem = colorItem(student.rfidCardId, "#3498DB");
        rfidItem->setFont(mono);
        t->setItem(r, 2, rfidItem);
        t->setItem(r, 3, new QTableWidgetItem(QString("Rs. %1").arg(student.balance)));
        t->setItem(r, 4, new QTableWidgetItem(student.registrationDate));
        t->setRowHeight(r, 40);
    }

    qDebug() << "Table updated with" << students.size() << "rows";
}

Student StudentsPage::getSelectedStudent()
{
    QTableWidget *t = ui->tableStudents;
    int row = t->currentRow();
    if (row < 0) return Student();

    QTableWidgetItem *rollItem = t->item(row, 1);
    if (!rollItem) return Student();

    return m_db->getStudentByRoll(rollItem->text());
}