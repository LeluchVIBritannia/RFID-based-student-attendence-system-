#ifndef STUDENTDASHBOARDPAGE_H
#define STUDENTDASHBOARDPAGE_H

#include <QWidget>

namespace Ui { class StudentDashboardPage; }

class StudentDashboardPage : public QWidget
{
    Q_OBJECT
public:
    explicit StudentDashboardPage(QWidget *parent = nullptr);
    ~StudentDashboardPage();

    // Call this when a card is tapped/scanned to refresh the view with
    // that student's live data (currently populates with demo values).
    void loadStudentByCardId(const QString &cardId);

private:
    void populateDemoData();

    Ui::StudentDashboardPage *ui;
};

#endif
