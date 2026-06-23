#ifndef STUDENTSPAGE_H
#define STUDENTSPAGE_H

#include <QWidget>
#include <QPushButton>
#include "database.h"

namespace Ui { class StudentsPage; }

class StudentsPage : public QWidget
{
    Q_OBJECT

public:
    explicit StudentsPage(QWidget *parent = nullptr);
    ~StudentsPage();

    void refreshData();

public slots:
    void onStudentAdded();
    void onDeleteStudentClicked();

signals:
    void navigateToRegister();

private:
    void connectButtons();
    void updateStudentTable();
    Student getSelectedStudent();

    Ui::StudentsPage *ui;
    DatabaseManager *m_db;
};

#endif