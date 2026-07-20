# RFID-Based Student Attendance and Cafeteria Management System


Department:Department of Computer Science and EngineeringInstitution:
Kathmandu University

## Team Members

| S.N. | Name             | Roll No. |
|------|------------------|----------|
| 1    | Krimud Sainju    | 08       |
| 2    | Alvin Shah       | 10       |
| 3    | Sanskar Shrestha | 23       |
| 4    | Pallav Thani     | 32       |
| 5    | Bishwo Timalsina | 37       |

Submitted to: Suman Shrestha



Abstract

This project presents the design and implementation of an RFID-based system for automating student attendance tracking and cafeteria payments within an academic institution. Each student is issued a unique RFID card, which serves as a single credential for both marking attendance and making cashless payments at the cafeteria. The system aims to reduce manual record-keeping errors, prevent proxy attendance, and streamline cafeteria transactions through real-time card verification and database logging.



1. Introduction

Traditional methods of taking attendance and managing cafeteria payments in institutions are time-consuming, prone to human error, and vulnerable to manipulation (e.g., proxy attendance). This project addresses these limitations by introducing a unified RFID-based solution that automates both processes using a single card per student, supported by embedded hardware and a desktop management application.


 2. Objectives

- To design and implement an automated attendance recording system using RFID technology.
- To enable cashless cafeteria transactions using the same RFID card.
- To prevent duplicate attendance entries within a single session.
- To provide real-time feedback to users (Attendance Marked / Already Recorded / Invalid Card).
- To generate attendance and expenditure reports for administrative use.



3. System Overview

Each student is assigned a unique RFID card used for two functions:

Attendance Recording — The card is scanned at the entry point; the system records the timestamp and prevents duplicate entries for the same session.
2. Cafeteria Payment — The same card is scanned at the cafeteria counter to deduct the purchase amount from the student's stored balance.

The system also generates reports summarizing attendance percentage and monthly cafeteria expenditure for administrative review.


 4. Tools and Technologies

| Layer    | Technology |
|----------|------------|
| Language | C++ (Qt Framework) |
| User Interface | Qt Designer (`.ui` files) |
| Database | MySQL (backend — not included in this repository) |
| Hardware | Audrino UNO , RFID Reader/Tags, LED, Buzzer |
| Build System | qmake / CMake |

---

 5. System Architecture and Workflow

```
Student scans RFID card
        |
        v
System reads Card ID
        |
   +----+-----+
   |          |
 Valid      Invalid
   |          |
   v          v
Record      Reject card
attendance  (LED + buzzer)
with
timestamp
   |
   v
Already recorded?
   |
   +-- Yes --> "Already Recorded" feedback
   |
   +-- No  --> "Attendance Marked" feedback
               + store to database

Cafeteria transaction:
   -> Deduct amount from student balance
   -> Store transaction record in database
```

---

6. Project Structure

```
rfid_system/
├── src/                        C++ source and header files
│   ├── main.cpp
│   ├── MainWindow.h/cpp        Application entry point / page router
│   ├── LoginPage.h/cpp         Login screen
│   ├── DashboardPage.h/cpp     Sidebar shell and navigation wiring
│   ├── OverviewPage.h/cpp      Dashboard overview
│   ├── AttendancePage.h/cpp    Attendance records and RFID scan handling
│   ├── CafeteriaPage.h/cpp     Cafeteria transactions
│   ├── StudentsPage.h/cpp      Student registry
│   ├── ReportsPage.h/cpp       Attendance percentage and expenditure reports
│   ├── RegisterCardPage.h/cpp  RFID card registration form
│   └── ScanTerminalPage.h/cpp  Kiosk scan feedback display
│
├── ui/                         Qt Designer UI files
│   ├── LoginPage.ui
│   ├── DashboardPage.ui
│   ├── OverviewPage.ui
│   ├── AttendancePage.ui
│   ├── CafeteriaPage.ui
│   ├── StudentsPage.ui
│   ├── ReportsPage.ui
│   ├── RegisterCardPage.ui
│   └── ScanTerminalPage.ui
│
├── rfid_system.pro             qmake build file
├── CMakeLists.txt              CMake build file (Qt6)
├── .gitignore
├── LICENSE
└── README.md



7. Module Description

| Module | Description |
|--------|--------------|
| Login | Secure admin login screen |
| Overview | Summary statistics and recent activity feed |
| Attendance | RFID scan terminal and daily attendance table |
| Cafeteria | Transaction log and daily revenue statistics |
| Students | Student registry with RFID card IDs and account balances |
| Reports | Attendance percentage report and monthly expenditure report |
| Register Card | Interface to assign a new RFID card to a student |
| Scan Terminal | Kiosk-style feedback display (Marked / Already Recorded / Invalid Card / Payment) |



 8. Build and Execution Instructions

#Prerequisites

- Qt 5.15+ or Qt 6.x with Widgets and Sql modules
- C++17-compliant compiler (GCC, Clang, or MSVC)
- qmake or CMake 3.16+

 Method A: Using qmake

```bash
qmake rfid_system.pro
make          # Linux/macOS
nmake         # Windows (MSVC)
```

 Method B: Using CMake

```bash
cmake -B build -DCMAKE_PREFIX_PATH=/path/to/Qt
cmake --build build
```

 Running the Application

```bash
./rfid_system        # Linux/macOS
rfid_system.exe      # Windows

Demo credentials:Username: `admin` | Password: `admin123`

Each `.ui` file in the `ui/` directory can also be opened independently in Qt Designer or Qt Creator (File → Open File). Qt's `uic` tool automatically generates the corresponding `ui_*.h` headers at build time; these generated files are not committed to the repository.

 9. Conclusion

The RFID-Based Student Attendance and Cafeteria Management System demonstrates a practical application of embedded systems and desktop software integration to solve a common institutional problem. By consolidating attendance tracking and cafeteria payment into a single RFID credential, the system reduces administrative overhead, minimizes errors associated with manual processes, and provides a scalable foundation for further extensions such as mobile notifications or web-based reporting.






 Institution

Kathmandu University
Department of Computer Science and Engineering
Dhulikhel, Kavre, Nepal
