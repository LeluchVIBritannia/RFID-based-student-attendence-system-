#include "MainWindow.h"
#include "SerialManager.h"
#include <QScreen>
#include <QGuiApplication>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // --- Database ---
    m_db = DatabaseManager::instance();
    if (!m_db->isOpen()) {
        m_db->initDatabase();
    }

    setWindowTitle("KU RFID — Student Attendance & Cafeteria System");
    setMinimumSize(1100, 700);

    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect geo = screen->availableGeometry();
        resize(int(geo.width() * 0.85), int(geo.height() * 0.85));
        move((geo.width() - width()) / 2, (geo.height() - height()) / 2);
    }

    // Stacked widget
    m_stack = new QStackedWidget(this);
    setCentralWidget(m_stack);

    //Create pages
    m_scanTerminalPage = new ScanTerminalPage(this);
    m_loginPage = new LoginPage(this);
    m_dashboardPage = new DashboardPage(this);
    m_studentDashboardPage = new StudentDashboardPage(this);

    m_dashboardPage->setDatabase(m_db);

    // Add to stack (order must match PageIndex enum)
    m_stack->addWidget(m_scanTerminalPage);          // index 0
    m_stack->addWidget(m_loginPage);                 // index 1
    m_stack->addWidget(m_dashboardPage);             // index 2
    m_stack->addWidget(m_studentDashboardPage);      // index 3

    // Connect login/logout
    connect(m_loginPage, &LoginPage::loginSuccess, this, &MainWindow::goToDashboard);
    connect(m_dashboardPage, &DashboardPage::logoutRequested, this, &MainWindow::goToLogin);

    // RFID connect to ScanTerminalPage slot with default mode
    m_serial = new SerialManager(this);
    connect(m_serial, &SerialManager::cardScanned, this,
            [this](const QString &uid) {
                m_scanTerminalPage->onRFIDScanned(uid, "attendance");
            });

    // timer (20s)
    m_idleTimer = new QTimer(this);
    m_idleTimer->setSingleShot(true);
    m_idleTimer->setInterval(20000);
    connect(m_idleTimer, &QTimer::timeout, this, &MainWindow::handleIdleTimeout);

    //Start on Login page
    m_stack->setCurrentIndex(LOGIN);

    qDebug() << "MainWindow setup complete (starts on Login, scan terminal ready)";
}

MainWindow::~MainWindow()
{
    qDebug() << "MainWindow destroyed";
}

void MainWindow::goToDashboard()
{
    qDebug() << "Going to admin dashboard";
    m_stack->setCurrentIndex(DASHBOARD);
    m_dashboardPage->refreshAllPages();
}

void MainWindow::goToLogin()
{
    qDebug() << "Going to login";
    m_idleTimer->stop();
    m_stack->setCurrentIndex(LOGIN);
}

void MainWindow::showStudentDashboard(const QString &rfidCardId)
{
    qDebug() << " Showing student dashboard for RFID:" << rfidCardId;
    if (!m_db->isOpen()) {
        m_db->initDatabase();
    }
    m_studentDashboardPage->loadStudentByCardId(rfidCardId);
    m_stack->setCurrentIndex(STUDENT_DASHBOARD);
    m_idleTimer->start();
}

void MainWindow::handleIdleTimeout()
{
    if (m_stack->currentIndex() == STUDENT_DASHBOARD) {
        qDebug() << "Timeout reached. Returning to Login page.";
        m_stack->setCurrentIndex(LOGIN);
    }
}