#include "DashboardPage.h"
#include "ui_DashboardPage.h"

#include "OverviewPage.h"
#include "AttendancePage.h"
#include "CafeteriaPage.h"
#include "StudentsPage.h"
#include "ReportsPage.h"
#include "RegisterCardPage.h"
#include "ScanTerminalPage.h"

#include <QDateTime>
#include <QPushButton>
#include <QDebug>

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DashboardPage)
    , m_db(nullptr)
    , m_overviewPage(nullptr)
    , m_attendancePage(nullptr)
    , m_cafeteriaPage(nullptr)
    , m_studentsPage(nullptr)
    , m_reportsPage(nullptr)
    , m_registerPage(nullptr)
    , m_scanPage(nullptr)
{
    ui->setupUi(this);
    ui->labelTopDate->setText(QDateTime::currentDateTime().toString("ddd, dd MMM yyyy"));
    setupPages();
    wireNav();
    connect(ui->pushButtonLogout, &QPushButton::clicked, this, &DashboardPage::logoutRequested);
}

DashboardPage::~DashboardPage()
{
    delete ui;
}

void DashboardPage::setDatabase(DatabaseManager *db)
{
    m_db = db;
    refreshAllPages();
}

void DashboardPage::refreshAllPages()
{
    if (!m_db) return;

    if (m_overviewPage) m_overviewPage->refreshData();
    if (m_attendancePage) m_attendancePage->refreshData();
    if (m_cafeteriaPage) m_cafeteriaPage->refreshData();
    if (m_studentsPage) m_studentsPage->refreshData();
    if (m_reportsPage) m_reportsPage->refreshData();
    if (m_registerPage) m_registerPage->refreshData();
    if (m_scanPage) m_scanPage->refreshData();
}

void DashboardPage::setupPages()
{
    m_overviewPage   = new OverviewPage(this);
    m_attendancePage = new AttendancePage(this);
    m_cafeteriaPage  = new CafeteriaPage(this);
    m_studentsPage   = new StudentsPage(this);
    m_reportsPage    = new ReportsPage(this);
    m_registerPage   = new RegisterCardPage(this);
    m_scanPage       = new ScanTerminalPage(this);

    connect(m_registerPage, &RegisterCardPage::studentAdded,
            m_studentsPage, &StudentsPage::onStudentAdded);

    ui->stackedWidget->addWidget(m_overviewPage);
    ui->stackedWidget->addWidget(m_attendancePage);
    ui->stackedWidget->addWidget(m_cafeteriaPage);
    ui->stackedWidget->addWidget(m_studentsPage);
    ui->stackedWidget->addWidget(m_reportsPage);
    ui->stackedWidget->addWidget(m_registerPage);
    ui->stackedWidget->addWidget(m_scanPage);

    ui->stackedWidget->setCurrentWidget(m_overviewPage);
}

void DashboardPage::wireNav()
{
    connect(ui->navBtnOverview,   &QPushButton::clicked, this, &DashboardPage::onNavClicked);
    connect(ui->navBtnAttendance, &QPushButton::clicked, this, &DashboardPage::onNavClicked);
    connect(ui->navBtnCafeteria,  &QPushButton::clicked, this, &DashboardPage::onNavClicked);
    connect(ui->navBtnStudents,   &QPushButton::clicked, this, &DashboardPage::onNavClicked);
    connect(ui->navBtnReports,    &QPushButton::clicked, this, &DashboardPage::onNavClicked);
    connect(ui->navBtnRegister,   &QPushButton::clicked, this, &DashboardPage::onNavClicked);
    connect(ui->navBtnScan,       &QPushButton::clicked, this, &DashboardPage::onNavClicked);
}

void DashboardPage::onNavClicked()
{
    QObject *sender_ = sender();

    ui->navBtnOverview->setChecked(sender_ == ui->navBtnOverview);
    ui->navBtnAttendance->setChecked(sender_ == ui->navBtnAttendance);
    ui->navBtnCafeteria->setChecked(sender_ == ui->navBtnCafeteria);
    ui->navBtnStudents->setChecked(sender_ == ui->navBtnStudents);
    ui->navBtnReports->setChecked(sender_ == ui->navBtnReports);
    ui->navBtnRegister->setChecked(sender_ == ui->navBtnRegister);
    ui->navBtnScan->setChecked(sender_ == ui->navBtnScan);

    if (sender_ == ui->navBtnOverview) {
        ui->stackedWidget->setCurrentWidget(m_overviewPage);
        updateOverviewPage();
    }
    else if (sender_ == ui->navBtnAttendance) {
        ui->stackedWidget->setCurrentWidget(m_attendancePage);
        updateAttendancePage();
    }
    else if (sender_ == ui->navBtnCafeteria) {
        ui->stackedWidget->setCurrentWidget(m_cafeteriaPage);
        updateCafeteriaPage();
    }
    else if (sender_ == ui->navBtnStudents) {
        ui->stackedWidget->setCurrentWidget(m_studentsPage);
        updateStudentsPage();
    }
    else if (sender_ == ui->navBtnReports) {
        ui->stackedWidget->setCurrentWidget(m_reportsPage);
        updateReportsPage();
    }
    else if (sender_ == ui->navBtnRegister) {
        ui->stackedWidget->setCurrentWidget(m_registerPage);
        updateRegisterCardPage();
    }
    else if (sender_ == ui->navBtnScan) {
        ui->stackedWidget->setCurrentWidget(m_scanPage);
        updateScanTerminalPage();
    }
}

void DashboardPage::navigateToRegisterPage()
{
    ui->stackedWidget->setCurrentWidget(m_registerPage);
    ui->navBtnRegister->setChecked(true);
    updateRegisterCardPage();
}

void DashboardPage::updateOverviewPage()
{
    if (m_overviewPage) m_overviewPage->refreshData();
}

void DashboardPage::updateAttendancePage()
{
    if (m_attendancePage) m_attendancePage->refreshData();
}

void DashboardPage::updateCafeteriaPage()
{
    if (m_cafeteriaPage) m_cafeteriaPage->refreshData();
}

void DashboardPage::updateStudentsPage()
{
    if (m_studentsPage) m_studentsPage->refreshData();
}

void DashboardPage::updateReportsPage()
{
    if (m_reportsPage) m_reportsPage->refreshData();
}

void DashboardPage::updateRegisterCardPage()
{
    if (m_registerPage) m_registerPage->refreshData();
}

void DashboardPage::updateScanTerminalPage()
{
    if (m_scanPage) m_scanPage->refreshData();
}