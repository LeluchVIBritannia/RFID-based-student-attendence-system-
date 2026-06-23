#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDate>
#include <QDateTime>
#include <QVariantMap>
#include <QDebug>
#include <QList>
#include <QMap>
#include <QRandomGenerator>

struct Student {
    int id;
    QString name;
    QString rollNo;
    QString rfidCardId;
    int balance;
    QString classSection;
    QString registrationDate;

    Student() : id(-1), balance(0) {}
};

struct AttendanceRecord {
    int id;
    int studentId;
    QString studentName;
    QString rollNo;
    QString date;
    QString timeIn;
    QString status;
    QString session;

    AttendanceRecord() : id(-1), studentId(-1) {}
};

struct CafeteriaTransaction {
    int id;
    int studentId;
    QString studentName;
    QString rollNo;
    QString item;
    int amount;
    QString date;
    QString time;

    CafeteriaTransaction() : id(-1), studentId(-1), amount(0) {}
};

struct Statistics {
    int totalStudents;
    int presentToday;
    int absentToday;
    int transactions;
    int revenue;
    int uniqueStudents;
    double avgAttendance;
    QString topAttendee;
    int totalMonthlySpend;

    Statistics() : totalStudents(0), presentToday(0), absentToday(0),
        transactions(0), revenue(0), uniqueStudents(0),
        avgAttendance(0.0), totalMonthlySpend(0) {}
};

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager* instance();
    ~DatabaseManager();

    bool initDatabase();
    bool isOpen() const { return m_db.isOpen(); }
    void closeDatabase();
    QSqlDatabase getDatabase() { return m_db; }  // ADD THIS

    // Student operations
    bool addStudent(const QString &name, const QString &rollNo,
                    const QString &rfidCardId, int balance,
                    const QString &classSection);
    bool updateStudent(const Student &student);
    bool deleteStudent(int studentId);
    bool deleteStudentByRoll(const QString &rollNo);
    QList<Student> getAllStudents();
    Student getStudentByRFID(const QString &rfidCardId);
    Student getStudentByRoll(const QString &rollNo);
    Student getStudentById(int id);
    bool updateStudentBalance(int studentId, int newBalance);
    int getStudentBalance(int studentId);
    void debugCheckDatabase();
    void closeAllQueries();

    // Attendance operations
    bool recordAttendance(int studentId, const QString &session);
    bool isAttendanceRecorded(int studentId, const QString &session);
    QList<AttendanceRecord> getTodaysAttendance();
    QList<AttendanceRecord> getAttendanceByDate(const QString &date);
    QList<AttendanceRecord> getAttendanceByStudent(int studentId);
    int getAttendanceCountByStudent(int studentId);

    // Cafeteria operations
    bool recordCafeteriaTransaction(int studentId, const QString &item, int amount);
    QList<CafeteriaTransaction> getTodaysTransactions();
    QList<CafeteriaTransaction> getTransactionsByDate(const QString &date);
    QList<CafeteriaTransaction> getTransactionsByStudent(int studentId);
    int getTotalTransactionsByStudent(int studentId);
    int getTotalSpentByStudent(int studentId);

    // Statistics and Reports
    Statistics getStatistics();
    Statistics getStatisticsForDate(const QString &date);
    QList<QMap<QString, QVariant>> getAttendanceReport();
    QList<QMap<QString, QVariant>> getCafeteriaReport();
    QList<QMap<QString, QVariant>> getStudentReport();

    QString getCurrentDate() const;
    QString getCurrentTime() const;

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    static DatabaseManager* m_instance;
    QSqlDatabase m_db;

    bool createTables();
    bool insertSampleData();
};

#endif // DATABASE_H