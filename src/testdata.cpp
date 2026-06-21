#include "testdata.h"
#include "database.h"
#include <QDebug>
#include <QRandomGenerator>
#include <QSqlQuery>
#include <QDate>
#include <QTime>

void createAttendanceRecords()
{
    qDebug() << "========================================";
    qDebug() << "Creating attendance records";
    qDebug() << "========================================";

    DatabaseManager* db = DatabaseManager::instance();

    if (!db->isOpen()) {
        qDebug() << "Database not open!";
        return;
    }

    QList<Student> students = db->getAllStudents();

    if (students.isEmpty()) {
        qDebug() << "No students found!";
        return;
    }

    qDebug() << "Found" << students.size() << "students";

    // Clear existing attendance
    QSqlQuery clear(db->getDatabase());
    clear.exec("DELETE FROM attendance");
    qDebug() << "Cleared attendance records";

    // Add 7 days of attendance for each student
    int totalAdded = 0;
    for (const Student &student : students) {
        for (int day = 0; day < 7; ++day) {
            QDate date = QDate::currentDate().addDays(-day);
            QString dateStr = date.toString("yyyy-MM-dd");

            QSqlQuery query(db->getDatabase());
            query.prepare("INSERT INTO attendance (student_id, date, time_in, status, session) "
                          "VALUES (?, ?, ?, ?, ?)");
            query.addBindValue(student.id);
            query.addBindValue(dateStr);
            query.addBindValue(QTime::currentTime().addSecs(-QRandomGenerator::global()->bounded(3600)).toString("hh:mm AP"));
            query.addBindValue("Present");
            query.addBindValue("Morning");

            if (query.exec()) {
                totalAdded++;
            }
        }
    }

    qDebug() << "Added" << totalAdded << "attendance records";

    QSqlQuery verify(db->getDatabase());
    if (verify.exec("SELECT COUNT(*) FROM attendance") && verify.next()) {
        qDebug() << "Total attendance records:" << verify.value(0).toInt();
    }

    qDebug() << "========================================";
}

void addCafeteriaTransactions()
{
    qDebug() << "========================================";
    qDebug() << "Adding cafeteria transactions";
    qDebug() << "========================================";

    DatabaseManager* db = DatabaseManager::instance();

    if (!db->isOpen()) {
        qDebug() << "Database not open!";
        return;
    }

    QList<Student> students = db->getAllStudents();

    if (students.isEmpty()) {
        qDebug() << "No students found!";
        return;
    }

    // Sample items - using simple struct approach
    struct MenuItem {
        QString name;
        int price;
    };

    QList<MenuItem> items = {
        {"Lunch Set", 180},
        {"Tea + Snack", 65},
        {"Coffee", 80},
        {"Pasta", 120},
        {"Pizza", 150},
        {"Sandwich", 100},
        {"Juice", 50},
        {"Burger", 130},
        {"Fries", 70}
    };

    // Clear existing transactions
    QSqlQuery clear(db->getDatabase());
    clear.exec("DELETE FROM cafeteria_transactions");
    qDebug() << "Cleared cafeteria transactions";

    int count = 0;
    for (const Student &student : students) {
        int numTransactions = QRandomGenerator::global()->bounded(2, 6);
        for (int i = 0; i < numTransactions; ++i) {
            const MenuItem &item = items[QRandomGenerator::global()->bounded(items.size())];
            QDate date = QDate::currentDate().addDays(-QRandomGenerator::global()->bounded(5));
            QTime time = QTime::currentTime().addSecs(-QRandomGenerator::global()->bounded(28800));

            QSqlQuery query(db->getDatabase());
            query.prepare("INSERT INTO cafeteria_transactions (student_id, item, amount, date, time) "
                          "VALUES (?, ?, ?, ?, ?)");
            query.addBindValue(student.id);
            query.addBindValue(item.name);
            query.addBindValue(item.price);
            query.addBindValue(date.toString("yyyy-MM-dd"));
            query.addBindValue(time.toString("hh:mm AP"));

            if (query.exec()) {
                count++;
                // Update balance
                int currentBalance = db->getStudentBalance(student.id);
                db->updateStudentBalance(student.id, currentBalance - item.price);
            }
        }
    }

    qDebug() << "Added" << count << "cafeteria transactions";

    QSqlQuery verify(db->getDatabase());
    if (verify.exec("SELECT COUNT(*) FROM cafeteria_transactions") && verify.next()) {
        qDebug() << "Total transactions:" << verify.value(0).toInt();
    }

    qDebug() << "========================================";
}

void addManualTestData()
{
    qDebug() << "========================================";
    qDebug() << "Adding custom test data";
    qDebug() << "========================================";

    DatabaseManager* db = DatabaseManager::instance();

    if (!db->isOpen()) {
        qDebug() << "Database not open!";
        return;
    }

    // Check if students already exist
    QSqlQuery check(db->getDatabase());
    if (check.exec("SELECT COUNT(*) FROM students") && check.next()) {
        int count = check.value(0).toInt();
        if (count > 0) {
            qDebug() << "Students already exist (" << count << "). Creating data...";
            createAttendanceRecords();
            addCafeteriaTransactions();
            return;
        }
    }

    // YOUR CUSTOM STUDENTS
    QList<QMap<QString, QString>> myStudents = {
        {{"name", "Ram Sainju"}, {"roll", "01"}, {"rfid", "123123"}, {"balance", "7740"}, {"class", "CE-I/II"}},
        {{"name", "Sita Sainju"}, {"roll", "02"}, {"rfid", "12345678"}, {"balance", "10230"}, {"class", "CE-I/II"}},
        {{"name", "Bob Johnson"}, {"roll", "03"}, {"rfid", "RFID003"}, {"balance", "5"}, {"class", "CSE-II/II"}},
        {{"name", "Alice Brown"}, {"roll", "04"}, {"rfid", "RFID004"}, {"balance", "20"}, {"class", "CSE-II/II"}},
        {{"name", "Charlie Davis"}, {"roll", "05"}, {"rfid", "RFID005"}, {"balance", "10"}, {"class", "CSE-III/II"}},
        {{"name", "David Wilson"}, {"roll", "06"}, {"rfid", "RFID006"}, {"balance", "495"}, {"class", "CSE-III/II"}},
        {{"name", "Emma Taylor"}, {"roll", "07"}, {"rfid", "RFID007"}, {"balance", "5"}, {"class", "CSE-I/II"}},
        {{"name", "Frank Miller"}, {"roll", "08"}, {"rfid", "RFID008"}, {"balance", "10"}, {"class", "CSE-II/II"}},
        {{"name", "Grace Lee"}, {"roll", "09"}, {"rfid", "RFID009"}, {"balance", "5"}, {"class", "CSE-III/II"}},
        {{"name", "Henry Clark"}, {"roll", "10"}, {"rfid", "RFID010"}, {"balance", "15"}, {"class", "CSE-I/II"}}
    };

    // Add students
    for (const auto &student : myStudents) {
        db->addStudent(student["name"],
                       student["roll"],
                       student["rfid"],
                       student["balance"].toInt(),
                       student["class"]);
    }

    qDebug() << "Added" << myStudents.size() << "students";

    // Create attendance and cafeteria transactions
    createAttendanceRecords();
    addCafeteriaTransactions();

    qDebug() << "========================================";
    qDebug() << "Custom data added successfully!";
    qDebug() << "========================================";
}