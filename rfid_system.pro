QT       += core gui widgets sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG   += c++17
TARGET    = rfid_system
TEMPLATE  = app
VERSION   = 1.0.0

SOURCES += \
    src/database.cpp \
    src/main.cpp \
    src/MainWindow.cpp \
    src/LoginPage.cpp \
    src/DashboardPage.cpp \
    src/OverviewPage.cpp \
    src/AttendancePage.cpp \
    src/CafeteriaPage.cpp \
    src/StudentsPage.cpp \
    src/ReportsPage.cpp \
    src/RegisterCardPage.cpp \
    src/ScanTerminalPage.cpp \
    src/testdata.cpp

HEADERS += \
    src/MainWindow.h \
    src/LoginPage.h \
    src/DashboardPage.h \
    src/OverviewPage.h \
    src/AttendancePage.h \
    src/CafeteriaPage.h \
    src/StudentsPage.h \
    src/ReportsPage.h \
    src/RegisterCardPage.h \
    src/ScanTerminalPage.h \
    src/database.h \
    src/testdata.h

FORMS += \
    ui/LoginPage.ui \
    ui/DashboardPage.ui \
    ui/OverviewPage.ui \
    ui/AttendancePage.ui \
    ui/CafeteriaPage.ui \
    ui/StudentsPage.ui \
    ui/ReportsPage.ui \
    ui/RegisterCardPage.ui \
    ui/ScanTerminalPage.ui

# Add this after QT += core widgets sql
INCLUDEPATH += $$[QT_INSTALL_HEADERS]