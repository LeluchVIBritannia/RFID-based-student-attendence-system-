#include "MainWindow.h"
#include "LoginPage.h"
#include "DashboardPage.h"
#include "StudentDashboardPage.h"  // ADD THIS
#include "StudentDashboardPage.h"
#include "SerialManager.h"
#include "testdata.h"

#include <QScreen>
#include <QGuiApplication>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    m_db = DatabaseManager::instance();
    qDebug() << "✅ Database manager created";
    qDebug() << "Database manager created";

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
    m_studentDashboardPage = new StudentDashboardPage(this);  // ADD THIS
    m_loginPage       = new LoginPage(this);
    m_dashboardPage   = new DashboardPage(this);
    m_studentViewPage = new StudentDashboardPage(this);

    // Pass database to dashboard page
    m_dashboardPage->setDatabase(m_db);

    // Add pages to stack
    m_stack->addWidget(m_loginPage);              // index 0 - LOGIN
    m_stack->addWidget(m_dashboardPage);          // index 1 - DASHBOARD
    m_stack->addWidget(m_studentDashboardPage);   // index 2 - STUDENT_DASHBOARD
    m_stack->addWidget(m_loginPage);       // index 0 = LOGIN
    m_stack->addWidget(m_dashboardPage);   // index 1 = DASHBOARD
    m_stack->addWidget(m_studentViewPage); // index 2 = STUDENT_VIEW

    // Connect signals
    connect(m_loginPage, &LoginPage::loginSuccess, this, &MainWindow::goToDashboard);
    connect(m_dashboardPage, &DashboardPage::logoutRequested, this, &MainWindow::goToLogin);

    // Start with login page
    m_stack->setCurrentIndex(LOGIN);

    qDebug() << "✅ MainWindow setup complete";
    // --- RFID wiring ---
    m_serial = new SerialManager(this);
    connect(m_serial, &SerialManager::cardScanned, this, &MainWindow::handleCardScanned);

    // --- idle timeout back to login/kiosk-wait screen ---
    m_idleTimer = new QTimer(this);
    m_idleTimer->setSingleShot(true);
    m_idleTimer->setInterval(20000); // 20s, tune as needed
    connect(m_idleTimer, &QTimer::timeout, this, &MainWindow::handleIdleTimeout);

    m_stack->setCurrentIndex(LOGIN);
    qDebug() << "MainWindow setup complete";
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
    qDebug() << "Going to login";
    m_idleTimer->stop();
    m_stack->setCurrentIndex(LOGIN);
}

void MainWindow::handleCardScanned(const QString &uid)
{
    qDebug() << "Card scanned:" << uid;

    // (Optional: feed the scan into DashboardPage's own ScanTerminalPage instead.)
    if (m_stack->currentIndex() == DASHBOARD) {
        qDebug() << "Ignoring kiosk card scan — admin dashboard is active";
        return;
    }

    Student student = m_db->getStudentByRFID(uid);
    if (student.id == -1) {
        qDebug() << "Unrecognized card:" << uid;
        // TODO: surface this on-screen — e.g. a shared "idle/status" label.
        // For now this could show a message on m_studentViewPage's own error state,
        // or a lightweight overlay you add to LOGIN page.
        return;
    }

    m_db->recordAttendance(student.id, "Morning");

    m_studentViewPage->loadStudentByCardId(uid);
    m_stack->setCurrentIndex(STUDENT_VIEW);

    m_idleTimer->start();
}

void MainWindow::handleIdleTimeout()
{
    if (m_stack->currentIndex() == STUDENT_VIEW) {
        m_stack->setCurrentIndex(LOGIN);
    }
}