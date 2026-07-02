cmake_minimum_required(VERSION 3.16)
project(rfid_system VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ui)


find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql SerialPort)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/LoginPage.cpp
    src/DashboardPage.cpp
    src/OverviewPage.cpp
    src/AttendancePage.cpp
    src/CafeteriaPage.cpp
    src/StudentsPage.cpp
    src/ReportsPage.cpp
    src/RegisterCardPage.cpp
    src/ScanTerminalPage.cpp
    src/StudentDashboardPage.cpp
)

set(HEADERS
    src/MainWindow.h
    src/LoginPage.h
    src/DashboardPage.h
    src/OverviewPage.h
    src/AttendancePage.h
    src/CafeteriaPage.h
    src/StudentsPage.h
    src/ReportsPage.h
    src/RegisterCardPage.h
    src/ScanTerminalPage.h
    src/StudentDashboardPage.h
)


set(FORMS
    ui/LoginPage.ui
    ui/DashboardPage.ui
    ui/OverviewPage.ui
    ui/AttendancePage.ui
    ui/CafeteriaPage.ui
    ui/StudentsPage.ui
    ui/ReportsPage.ui
    ui/RegisterCardPage.ui
    ui/ScanTerminalPage.ui
    ui/StudentDashboardPage.ui
)

qt_add_executable(rfid_system ${SOURCES} ${HEADERS} ${FORMS}
    src/SerialManager.h
    src/SerialMangaer.cpp)


target_link_libraries(rfid_system PRIVATE
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets 
    Qt6::Sql 
    Qt6::SerialPort
)


set_target_properties(rfid_system PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)