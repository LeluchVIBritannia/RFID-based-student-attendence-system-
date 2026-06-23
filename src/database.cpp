#include "database.h"
#include <QSqlRecord>
#include <QSqlField>
#include <QThread>
#include <QSqlQuery>
#include <QtGlobal>
#include <QSqlError>
#include <QSqlDriver>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QDebug>
#include <QRandomGenerator>
#include <QCoreApplication>

DatabaseManager* DatabaseManager::m_instance = nullptr;

DatabaseManager* DatabaseManager::instance()
{
    if (!m_instance) {
        m_instance = new DatabaseManager();
    }
    return m_instance;
}

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent)
{
    initDatabase();
}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();
    m_instance = nullptr;
}

bool DatabaseManager::initDatabase()
{
    // Set the plugin path for SQL drivers
    QString appPath = QCoreApplication::applicationDirPath();
    QCoreApplication::addLibraryPath(appPath);
    QCoreApplication::addLibraryPath(appPath + "/sqldrivers");

    // Check available drivers
    QStringList drivers = QSqlDatabase::drivers();
    qDebug() << "Available SQL drivers:" << drivers;

    if (!drivers.contains("QSQLITE")) {
        qDebug() << "QSQLITE driver not found!";
        qDebug() << "Application path:" << appPath;
        qDebug() << "Library paths:" << QCoreApplication::libraryPaths();
    }

    QString dbPath = QDir::currentPath() + "/ku_rfid.db";
    qDebug() << "========================================";
    qDebug() << "Database path:" << dbPath;
    qDebug() << "File exists:" << QFile::exists(dbPath);
    qDebug() << "========================================";

    if (QSqlDatabase::contains("ku_rfid_connection")) {
        m_db = QSqlDatabase::database("ku_rfid_connection");
        if (m_db.isOpen()) {
            qDebug() << "Using existing database connection";
            return true;
        }
        QSqlDatabase::removeDatabase("ku_rfid_connection");
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE", "ku_rfid_connection");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qDebug() << "Error opening database:" << m_db.lastError().text();
        return false;
    }

    qDebug() << "Database opened successfully";

    QSqlQuery pragmaQuery("PRAGMA foreign_keys = ON;", m_db);
    if (!pragmaQuery.exec()) {
        qDebug() << "Failed to enable foreign keys:" << pragmaQuery.lastError().text();
    }

    QSqlQuery timeoutQuery("PRAGMA busy_timeout = 10000;", m_db);
    timeoutQuery.exec();

    if (!createTables()) {
        qDebug() << "Error creating tables";
        return false;
    }

    // Just check and display student count - NO AUTO DATA INSERT
    QSqlQuery countQuery("SELECT COUNT(*) FROM students", m_db);
    if (countQuery.exec() && countQuery.next()) {
        int count = countQuery.value(0).toInt();
        qDebug() << "Database has" << count << "students";

        if (count > 0) {
            QSqlQuery listQuery("SELECT id, name, roll_no FROM students", m_db);
            while (listQuery.next()) {
                qDebug() << "   - ID:" << listQuery.value("id").toInt()
                << "Name:" << listQuery.value("name").toString()
                << "Roll:" << listQuery.value("roll_no").toString();
            }
        } else {
            qDebug() << "Database is empty. Use testdata.cpp to add data.";
        }
    }

    return true;
}

void DatabaseManager::closeDatabase()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    if (QSqlDatabase::contains("ku_rfid_connection")) {
        QSqlDatabase::removeDatabase("ku_rfid_connection");
    }
}

void DatabaseManager::closeAllQueries()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    if (QSqlDatabase::contains("ku_rfid_connection")) {
        QSqlDatabase::removeDatabase("ku_rfid_connection");
    }
    initDatabase();
}

void DatabaseManager::debugCheckDatabase()
{
    qDebug() << "=== Database Debug Info ===";
    if (!m_db.isOpen()) {
        qDebug() << "Database is NOT open!";
        return;
    }

    QSqlQuery count("SELECT COUNT(*) FROM students", m_db);
    if (count.next()) {
        qDebug() << "Total students:" << count.value(0).toInt();
    }

    QSqlQuery list("SELECT id, name, roll_no FROM students", m_db);
    while (list.next()) {
        qDebug() << "  - ID:" << list.value(0).toInt()
        << "Name:" << list.value(1).toString()
        << "Roll:" << list.value(2).toString();
    }
}

bool DatabaseManager::createTables()
{
    if (!m_db.isOpen()) {
        qDebug() << "Database is not open in createTables!";
        return false;
    }

    QSqlQuery query(m_db);
    query.exec("PRAGMA foreign_keys = ON;");

    bool success = query.exec(
        "CREATE TABLE IF NOT EXISTS students ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL, "
        "roll_no TEXT UNIQUE NOT NULL, "
        "rfid_card_id TEXT UNIQUE NOT NULL, "
        "balance INTEGER DEFAULT 0, "
        "class_section TEXT, "
        "registration_date TEXT"
        ")"
        );

    if (!success) {
        qDebug() << "Error creating students table:" << query.lastError().text();
        return false;
    }

    success = query.exec(
        "CREATE TABLE IF NOT EXISTS attendance ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "student_id INTEGER NOT NULL, "
        "date TEXT NOT NULL, "
        "time_in TEXT NOT NULL, "
        "status TEXT DEFAULT 'Present', "
        "session TEXT, "
        "FOREIGN KEY (student_id) REFERENCES students (id) ON DELETE CASCADE, "
        "UNIQUE(student_id, date, session)"
        ")"
        );

    if (!success) {
        qDebug() << "Error creating attendance table:" << query.lastError().text();
        return false;
    }

    success = query.exec(
        "CREATE TABLE IF NOT EXISTS cafeteria_transactions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "student_id INTEGER NOT NULL, "
        "item TEXT NOT NULL, "
        "amount INTEGER NOT NULL, "
        "date TEXT NOT NULL, "
        "time TEXT NOT NULL, "
        "FOREIGN KEY (student_id) REFERENCES students (id) ON DELETE CASCADE"
        ")"
        );

    if (!success) {
        qDebug() << "Error creating cafeteria_transactions table:" << query.lastError().text();
        return false;
    }

    qDebug() << "Tables created successfully";
    return true;
}


bool DatabaseManager::addStudent(const QString &name, const QString &rollNo,
                                 const QString &rfidCardId, int balance,
                                 const QString &classSection)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO students (name, roll_no, rfid_card_id, balance, class_section, registration_date) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(name);
    query.addBindValue(rollNo);
    query.addBindValue(rfidCardId);
    query.addBindValue(balance);
    query.addBindValue(classSection);
    query.addBindValue(getCurrentDate());

    if (!query.exec()) {
        qDebug() << "Error adding student:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::updateStudent(const Student &student)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE students SET name = ?, roll_no = ?, rfid_card_id = ?, "
                  "balance = ?, class_section = ? WHERE id = ?");
    query.addBindValue(student.name);
    query.addBindValue(student.rollNo);
    query.addBindValue(student.rfidCardId);
    query.addBindValue(student.balance);
    query.addBindValue(student.classSection);
    query.addBindValue(student.id);

    if (!query.exec()) {
        qDebug() << "Error updating student:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::deleteStudent(int studentId)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database is not open!";
        return false;
    }

    QSqlQuery checkQuery(m_db);
    checkQuery.prepare("SELECT id, name FROM students WHERE id = ?");
    checkQuery.addBindValue(studentId);

    if (!checkQuery.exec() || !checkQuery.next()) {
        return false;
    }

    QSqlQuery deleteQuery(m_db);
    deleteQuery.prepare("DELETE FROM students WHERE id = ?");
    deleteQuery.addBindValue(studentId);

    if (!deleteQuery.exec()) {
        return false;
    }

    return deleteQuery.numRowsAffected() > 0;
}

bool DatabaseManager::deleteStudentByRoll(const QString &rollNo)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM students WHERE roll_no = ?");
    query.addBindValue(rollNo);

    if (!query.exec()) {
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<Student> DatabaseManager::getAllStudents()
{
    QList<Student> students;
    QSqlQuery query("SELECT id, name, roll_no, rfid_card_id, balance, class_section, registration_date "
                    "FROM students ORDER BY roll_no", m_db);

    while (query.next()) {
        Student student;
        student.id = query.value("id").toInt();
        student.name = query.value("name").toString();
        student.rollNo = query.value("roll_no").toString();
        student.rfidCardId = query.value("rfid_card_id").toString();
        student.balance = query.value("balance").toInt();
        student.classSection = query.value("class_section").toString();
        student.registrationDate = query.value("registration_date").toString();
        students.append(student);
    }

    return students;
}

Student DatabaseManager::getStudentByRFID(const QString &rfidCardId)
{
    Student student;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, name, roll_no, rfid_card_id, balance, class_section, registration_date "
                  "FROM students WHERE rfid_card_id = ?");
    query.addBindValue(rfidCardId);

    if (query.exec() && query.next()) {
        student.id = query.value("id").toInt();
        student.name = query.value("name").toString();
        student.rollNo = query.value("roll_no").toString();
        student.rfidCardId = query.value("rfid_card_id").toString();
        student.balance = query.value("balance").toInt();
        student.classSection = query.value("class_section").toString();
        student.registrationDate = query.value("registration_date").toString();
    }

    return student;
}

Student DatabaseManager::getStudentByRoll(const QString &rollNo)
{
    Student student;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, name, roll_no, rfid_card_id, balance, class_section, registration_date "
                  "FROM students WHERE roll_no = ?");
    query.addBindValue(rollNo);

    if (query.exec() && query.next()) {
        student.id = query.value("id").toInt();
        student.name = query.value("name").toString();
        student.rollNo = query.value("roll_no").toString();
        student.rfidCardId = query.value("rfid_card_id").toString();
        student.balance = query.value("balance").toInt();
        student.classSection = query.value("class_section").toString();
        student.registrationDate = query.value("registration_date").toString();
    }

    return student;
}

Student DatabaseManager::getStudentById(int id)
{
    Student student;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, name, roll_no, rfid_card_id, balance, class_section, registration_date "
                  "FROM students WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        student.id = query.value("id").toInt();
        student.name = query.value("name").toString();
        student.rollNo = query.value("roll_no").toString();
        student.rfidCardId = query.value("rfid_card_id").toString();
        student.balance = query.value("balance").toInt();
        student.classSection = query.value("class_section").toString();
        student.registrationDate = query.value("registration_date").toString();
    }

    return student;
}

bool DatabaseManager::updateStudentBalance(int studentId, int newBalance)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE students SET balance = ? WHERE id = ?");
    query.addBindValue(newBalance);
    query.addBindValue(studentId);

    if (!query.exec()) {
        qDebug() << "Error updating student balance:" << query.lastError().text();
        return false;
    }

    return true;
}

int DatabaseManager::getStudentBalance(int studentId)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT balance FROM students WHERE id = ?");
    query.addBindValue(studentId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

bool DatabaseManager::recordAttendance(int studentId, const QString &session)
{
    if (isAttendanceRecorded(studentId, session)) {
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO attendance (student_id, date, time_in, status, session) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(studentId);
    query.addBindValue(getCurrentDate());
    query.addBindValue(getCurrentTime());
    query.addBindValue("Present");
    query.addBindValue(session);

    if (!query.exec()) {
        qDebug() << "Error recording attendance:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::isAttendanceRecorded(int studentId, const QString &session)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM attendance WHERE student_id = ? AND date = ? AND session = ?");
    query.addBindValue(studentId);
    query.addBindValue(getCurrentDate());
    query.addBindValue(session);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

QList<AttendanceRecord> DatabaseManager::getTodaysAttendance()
{
    return getAttendanceByDate(getCurrentDate());
}

QList<AttendanceRecord> DatabaseManager::getAttendanceByDate(const QString &date)
{
    QList<AttendanceRecord> records;
    QSqlQuery query(m_db);
    query.prepare("SELECT a.*, s.name as student_name, s.roll_no "
                  "FROM attendance a "
                  "JOIN students s ON a.student_id = s.id "
                  "WHERE a.date = ? "
                  "ORDER BY a.time_in DESC");
    query.addBindValue(date);

    if (query.exec()) {
        while (query.next()) {
            AttendanceRecord record;
            record.id = query.value("id").toInt();
            record.studentId = query.value("student_id").toInt();
            record.studentName = query.value("student_name").toString();
            record.rollNo = query.value("roll_no").toString();
            record.date = query.value("date").toString();
            record.timeIn = query.value("time_in").toString();
            record.status = query.value("status").toString();
            record.session = query.value("session").toString();
            records.append(record);
        }
    }

    return records;
}

QList<AttendanceRecord> DatabaseManager::getAttendanceByStudent(int studentId)
{
    QList<AttendanceRecord> records;
    QSqlQuery query(m_db);
    query.prepare("SELECT a.*, s.name as student_name, s.roll_no "
                  "FROM attendance a "
                  "JOIN students s ON a.student_id = s.id "
                  "WHERE a.student_id = ? "
                  "ORDER BY a.date DESC, a.time_in DESC");
    query.addBindValue(studentId);

    if (query.exec()) {
        while (query.next()) {
            AttendanceRecord record;
            record.id = query.value("id").toInt();
            record.studentId = query.value("student_id").toInt();
            record.studentName = query.value("student_name").toString();
            record.rollNo = query.value("roll_no").toString();
            record.date = query.value("date").toString();
            record.timeIn = query.value("time_in").toString();
            record.status = query.value("status").toString();
            record.session = query.value("session").toString();
            records.append(record);
        }
    }

    return records;
}

int DatabaseManager::getAttendanceCountByStudent(int studentId)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM attendance WHERE student_id = ?");
    query.addBindValue(studentId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

bool DatabaseManager::recordCafeteriaTransaction(int studentId, const QString &item, int amount)
{
    int balance = getStudentBalance(studentId);
    if (balance < amount) {
        return false;
    }

    if (!updateStudentBalance(studentId, balance - amount)) {
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO cafeteria_transactions (student_id, item, amount, date, time) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(studentId);
    query.addBindValue(item);
    query.addBindValue(amount);
    query.addBindValue(getCurrentDate());
    query.addBindValue(getCurrentTime());

    if (!query.exec()) {
        qDebug() << "Error recording cafeteria transaction:" << query.lastError().text();
        return false;
    }

    return true;
}

QList<CafeteriaTransaction> DatabaseManager::getTodaysTransactions()
{
    return getTransactionsByDate(getCurrentDate());
}

QList<CafeteriaTransaction> DatabaseManager::getTransactionsByDate(const QString &date)
{
    QList<CafeteriaTransaction> transactions;
    QSqlQuery query(m_db);
    query.prepare("SELECT c.*, s.name as student_name, s.roll_no "
                  "FROM cafeteria_transactions c "
                  "JOIN students s ON c.student_id = s.id "
                  "WHERE c.date = ? "
                  "ORDER BY c.time DESC");
    query.addBindValue(date);

    if (query.exec()) {
        while (query.next()) {
            CafeteriaTransaction transaction;
            transaction.id = query.value("id").toInt();
            transaction.studentId = query.value("student_id").toInt();
            transaction.studentName = query.value("student_name").toString();
            transaction.rollNo = query.value("roll_no").toString();
            transaction.item = query.value("item").toString();
            transaction.amount = query.value("amount").toInt();
            transaction.date = query.value("date").toString();
            transaction.time = query.value("time").toString();
            transactions.append(transaction);
        }
    }

    return transactions;
}

QList<CafeteriaTransaction> DatabaseManager::getTransactionsByStudent(int studentId)
{
    QList<CafeteriaTransaction> transactions;
    QSqlQuery query(m_db);
    query.prepare("SELECT c.*, s.name as student_name, s.roll_no "
                  "FROM cafeteria_transactions c "
                  "JOIN students s ON c.student_id = s.id "
                  "WHERE c.student_id = ? "
                  "ORDER BY c.date DESC, c.time DESC");
    query.addBindValue(studentId);

    if (query.exec()) {
        while (query.next()) {
            CafeteriaTransaction transaction;
            transaction.id = query.value("id").toInt();
            transaction.studentId = query.value("student_id").toInt();
            transaction.studentName = query.value("student_name").toString();
            transaction.rollNo = query.value("roll_no").toString();
            transaction.item = query.value("item").toString();
            transaction.amount = query.value("amount").toInt();
            transaction.date = query.value("date").toString();
            transaction.time = query.value("time").toString();
            transactions.append(transaction);
        }
    }

    return transactions;
}

int DatabaseManager::getTotalTransactionsByStudent(int studentId)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM cafeteria_transactions WHERE student_id = ?");
    query.addBindValue(studentId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

int DatabaseManager::getTotalSpentByStudent(int studentId)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT COALESCE(SUM(amount), 0) FROM cafeteria_transactions WHERE student_id = ?");
    query.addBindValue(studentId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

Statistics DatabaseManager::getStatistics()
{
    return getStatisticsForDate(getCurrentDate());
}

Statistics DatabaseManager::getStatisticsForDate(const QString &date)
{
    Statistics stats;

    QSqlQuery query("SELECT COUNT(*) FROM students", m_db);
    if (query.exec() && query.next()) {
        stats.totalStudents = query.value(0).toInt();
    }

    query.prepare("SELECT COUNT(DISTINCT student_id) FROM attendance WHERE date = ?");
    query.addBindValue(date);
    if (query.exec() && query.next()) {
        stats.presentToday = query.value(0).toInt();
    }

    stats.absentToday = stats.totalStudents - stats.presentToday;

    query.prepare("SELECT COUNT(*) FROM cafeteria_transactions WHERE date = ?");
    query.addBindValue(date);
    if (query.exec() && query.next()) {
        stats.transactions = query.value(0).toInt();
    }

    query.prepare("SELECT COALESCE(SUM(amount), 0) FROM cafeteria_transactions WHERE date = ?");
    query.addBindValue(date);
    if (query.exec() && query.next()) {
        stats.revenue = query.value(0).toInt();
    }

    query.prepare("SELECT COUNT(DISTINCT student_id) FROM cafeteria_transactions WHERE date = ?");
    query.addBindValue(date);
    if (query.exec() && query.next()) {
        stats.uniqueStudents = query.value(0).toInt();
    }

    query.prepare("SELECT COUNT(DISTINCT date) as total_days, COUNT(*) as total_attendance FROM attendance");
    if (query.exec() && query.next()) {
        int totalDays = query.value(0).toInt();
        int totalAttendance = query.value(1).toInt();
        if (totalDays > 0 && stats.totalStudents > 0) {
            stats.avgAttendance = (static_cast<double>(totalAttendance) / (stats.totalStudents * totalDays)) * 100;
        }
    }

    query.exec("SELECT s.name, COUNT(*) as count "
               "FROM attendance a "
               "JOIN students s ON a.student_id = s.id "
               "GROUP BY s.name "
               "ORDER BY count DESC "
               "LIMIT 1");
    if (query.next()) {
        stats.topAttendee = query.value(0).toString();
    }

    QString currentMonth = QDate::currentDate().toString("yyyy-MM");
    query.prepare("SELECT COALESCE(SUM(amount), 0) FROM cafeteria_transactions WHERE date LIKE ?");
    query.addBindValue(currentMonth + "%");
    if (query.exec() && query.next()) {
        stats.totalMonthlySpend = query.value(0).toInt();
    }

    return stats;
}

QList<QMap<QString, QVariant>> DatabaseManager::getAttendanceReport()
{
    QList<QMap<QString, QVariant>> report;

    QSqlQuery query("SELECT COUNT(DISTINCT date) FROM attendance", m_db);
    int totalClasses = 0;
    if (query.exec() && query.next()) {
        totalClasses = query.value(0).toInt();
    }

    query.exec("SELECT s.id, s.name, s.roll_no, COUNT(a.id) as attended "
               "FROM students s "
               "LEFT JOIN attendance a ON s.id = a.student_id "
               "GROUP BY s.id "
               "ORDER BY s.roll_no");

    while (query.next()) {
        QMap<QString, QVariant> row;
        row["id"] = query.value("id").toInt();
        row["name"] = query.value("name").toString();
        row["roll_no"] = query.value("roll_no").toString();
        row["attended"] = query.value("attended").toInt();
        row["total_classes"] = totalClasses;
        row["attendance_percentage"] = (totalClasses > 0) ? (query.value("attended").toInt() * 100 / totalClasses) : 0;
        report.append(row);
    }

    return report;
}

QList<QMap<QString, QVariant>> DatabaseManager::getCafeteriaReport()
{
    QList<QMap<QString, QVariant>> report;

    QSqlQuery query("SELECT s.id, s.name, s.roll_no, "
                    "COUNT(c.id) as transactions, "
                    "COALESCE(SUM(c.amount), 0) as total_spent, "
                    "s.balance "
                    "FROM students s "
                    "LEFT JOIN cafeteria_transactions c ON s.id = c.student_id "
                    "GROUP BY s.id "
                    "ORDER BY s.roll_no", m_db);

    while (query.next()) {
        QMap<QString, QVariant> row;
        row["id"] = query.value("id").toInt();
        row["name"] = query.value("name").toString();
        row["roll_no"] = query.value("roll_no").toString();
        row["transactions"] = query.value("transactions").toInt();
        row["total_spent"] = query.value("total_spent").toInt();
        row["balance"] = query.value("balance").toInt();
        report.append(row);
    }

    return report;
}

QList<QMap<QString, QVariant>> DatabaseManager::getStudentReport()
{
    QList<QMap<QString, QVariant>> report;

    QSqlQuery query("SELECT id, name, roll_no, rfid_card_id, balance, class_section, registration_date "
                    "FROM students ORDER BY roll_no", m_db);

    while (query.next()) {
        QMap<QString, QVariant> row;
        row["id"] = query.value("id").toInt();
        row["name"] = query.value("name").toString();
        row["roll_no"] = query.value("roll_no").toString();
        row["rfid_card_id"] = query.value("rfid_card_id").toString();
        row["balance"] = query.value("balance").toInt();
        row["class_section"] = query.value("class_section").toString();
        row["registration_date"] = query.value("registration_date").toString();
        report.append(row);
    }

    return report;
}

QString DatabaseManager::getCurrentDate() const
{
    return QDate::currentDate().toString("yyyy-MM-dd");
}

QString DatabaseManager::getCurrentTime() const
{
    return QTime::currentTime().toString("hh:mm AP");
}