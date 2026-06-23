#include "MainWindow.h"
#include "LoginPage.h"
#include "DashboardPage.h"
#include "testdata.h"

#include <QScreen>
#include <QGuiApplication>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // Initialize database
    m_db = DatabaseManager::instance();
    qDebug() << "Database manager created";

    // Add test data (students, attendance, cafeteria)
    addManualTestData();

    setWindowTitle("KU RFID — Student Attendance & Cafeteria System");
    setMinimumSize(1100, 700);

    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect geo = screen->availableGeometry();
        resize(int(geo.width() * 0.85), int(geo.height() * 0.85));
        move((geo.width() - width()) / 2, (geo.height() - height()) / 2);
    }

    m_stack = new QStackedWidget(this);
    setCentralWidget(m_stack);

    m_loginPage = new LoginPage(this);
    m_dashboardPage = new DashboardPage(this);

    m_dashboardPage->setDatabase(m_db);

    m_stack->addWidget(m_loginPage);
    m_stack->addWidget(m_dashboardPage);

    connect(m_loginPage, &LoginPage::loginSuccess, this, &MainWindow::goToDashboard);
    connect(m_dashboardPage, &DashboardPage::logoutRequested, this, &MainWindow::goToLogin);

    m_stack->setCurrentIndex(LOGIN);

    qDebug() << "MainWindow setup complete";
}

MainWindow::~MainWindow()
{
    qDebug() << "MainWindow destroyed";
}

void MainWindow::goToDashboard()
{
    qDebug() << "Going to dashboard";
    m_stack->setCurrentIndex(DASHBOARD);
    m_dashboardPage->refreshAllPages();
}

void MainWindow::goToLogin()
{
    qDebug() << "Going to login";
    m_stack->setCurrentIndex(LOGIN);
}