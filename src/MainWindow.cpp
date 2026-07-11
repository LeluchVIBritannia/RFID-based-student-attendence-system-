#include "MainWindow.h"
#include "LoginPage.h"
#include "DashboardPage.h"
#include "StudentDashboardPage.h"
#include "SerialManager.h"

#include <QScreen>
#include <QGuiApplication>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    m_db = DatabaseManager::instance();
    qDebug() << "✅ Database manager created";

#ifdef ENABLE_TEST_DATA
    addManualTestData();
#endif

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
    m_studentDashboardPage = new StudentDashboardPage(this);

    // Pass database to dashboard page
    m_dashboardPage->setDatabase(m_db);

    // Add pages to stack
    m_stack->addWidget(m_loginPage);              // index 0 - LOGIN
    m_stack->addWidget(m_dashboardPage);          // index 1 - DASHBOARD
    m_stack->addWidget(m_studentDashboardPage);   // index 2 - STUDENT_DASHBOARD

    // Connect signals
    connect(m_loginPage, &LoginPage::loginSuccess, this, &MainWindow::goToDashboard);
    connect(m_dashboardPage, &DashboardPage::logoutRequested, this, &MainWindow::goToLogin);

    // --- RFID wiring ---
    m_serial = new SerialManager(this);
    connect(m_serial, &SerialManager::cardScanned, this, &MainWindow::handleCardScanned);

    // --- idle timeout back to login/kiosk-wait screen ---
    m_idleTimer = new QTimer(this);
    m_idleTimer->setSingleShot(true);
    m_idleTimer->setInterval(20000); // 20s, tune as needed
    connect(m_idleTimer, &QTimer::timeout, this, &MainWindow::handleIdleTimeout);

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
    m_idleTimer->stop();
    m_stack->setCurrentIndex(LOGIN);
}

void MainWindow::showStudentDashboard(const QString &rfidCardId)
{
    qDebug() << "🔄 Showing student dashboard for RFID:" << rfidCardId;
    m_studentDashboardPage->loadStudentByCardId(rfidCardId);
    m_stack->setCurrentIndex(STUDENT_DASHBOARD);
    m_idleTimer->start();
}

void MainWindow::handleCardScanned(const QString &uid)
{
    qDebug() << "Card scanned:" << uid;

    // Ignore kiosk scans while admin is working in the dashboard
    if (m_stack->currentIndex() == DASHBOARD) {
        qDebug() << "Ignoring kiosk card scan — admin dashboard is active";
        return;
    }

    Student student = m_db->getStudentByRFID(uid);
    if (student.id == -1) {
        qDebug() << "Unrecognized card:" << uid;
        // TODO: surface this on-screen instead of just logging it
        return;
    }

    m_db->recordAttendance(student.id, "Morning");
    showStudentDashboard(uid);
}

void MainWindow::handleIdleTimeout()
{
    if (m_stack->currentIndex() == STUDENT_DASHBOARD) {
        qDebug() << "⏰ Kiosk timeout reached. Returning to Login screen.";
        m_stack->setCurrentIndex(LOGIN);
    }
}