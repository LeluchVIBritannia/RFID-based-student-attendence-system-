#include "MainWindow.h"
#include "LoginPage.h"
#include "DashboardPage.h"
#include "StudentDashboardPage.h"  // ADD THIS

#include <QScreen>
#include <QGuiApplication>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // Initialize database
    m_db = DatabaseManager::instance();
    qDebug() << "✅ Database manager created";

    setWindowTitle("KU RFID — Student Attendance & Cafeteria System");
    setMinimumSize(1100, 700);

    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect geo = screen->availableGeometry();
        resize(int(geo.width() * 0.85), int(geo.height() * 0.85));
        move((geo.width() - width()) / 2, (geo.height() - height()) / 2);
    }

    // Create stacked widget
    m_stack = new QStackedWidget(this);
    setCentralWidget(m_stack);

    // Create all pages
    m_loginPage = new LoginPage(this);
    m_dashboardPage = new DashboardPage(this);
    m_studentDashboardPage = new StudentDashboardPage(this);  // ADD THIS

    // Pass database to dashboard page
    m_dashboardPage->setDatabase(m_db);

    // Add pages to stack
    m_stack->addWidget(m_loginPage);              // index 0 - LOGIN
    m_stack->addWidget(m_dashboardPage);          // index 1 - DASHBOARD
    m_stack->addWidget(m_studentDashboardPage);   // index 2 - STUDENT_DASHBOARD

    // Connect signals
    connect(m_loginPage, &LoginPage::loginSuccess, this, &MainWindow::goToDashboard);
    connect(m_dashboardPage, &DashboardPage::logoutRequested, this, &MainWindow::goToLogin);

    // Start with login page
    m_stack->setCurrentIndex(LOGIN);

    qDebug() << "✅ MainWindow setup complete";
}

MainWindow::~MainWindow()
{
    qDebug() << "🗑️ MainWindow destroyed";
}

void MainWindow::goToDashboard()
{
    qDebug() << "🔄 Going to admin dashboard";
    m_stack->setCurrentIndex(DASHBOARD);
    m_dashboardPage->refreshAllPages();
}

void MainWindow::goToLogin()
{
    qDebug() << "🔄 Going to login";
    m_stack->setCurrentIndex(LOGIN);
}

// NEW: Show student dashboard when RFID is scanned
void MainWindow::showStudentDashboard(const QString &rfidCardId)
{
    qDebug() << "🔄 Showing student dashboard for RFID:" << rfidCardId;
    m_studentDashboardPage->loadStudentByCardId(rfidCardId);
    m_stack->setCurrentIndex(STUDENT_DASHBOARD);
}