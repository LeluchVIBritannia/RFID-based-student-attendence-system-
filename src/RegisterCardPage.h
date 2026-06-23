#ifndef REGISTERCARDPAGE_H
#define REGISTERCARDPAGE_H

#include <QWidget>
#include "database.h"

namespace Ui { class RegisterCardPage; }

class RegisterCardPage : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterCardPage(QWidget *parent = nullptr);
    ~RegisterCardPage();

    void refreshData();

signals:
    void studentAdded();

private slots:
    void onSaveStudent();
    void onDeleteStudent();
    void onScanCard();
    void onClearForm();

private:
    void wireForm();
    void clearForm();
    bool validateForm();
    bool saveStudent();
    Student getStudentFromForm();

    Ui::RegisterCardPage *ui;
    DatabaseManager *m_db;
    int m_currentStudentId;
};

#endif