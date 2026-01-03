# Qt 多平台行事曆整合工具
QT += core gui network sql widgets

# NetworkAuth module (Qt6::NetworkAuth) is REQUIRED for this project
# The code uses OAuth2 authentication which depends on NetworkAuth headers
# 
# If you get compilation errors about missing QOAuth headers, you need to install Qt NetworkAuth:
# 
# Windows:
#   1. Open Qt Maintenance Tool
#   2. Select "Add or remove components"
#   3. Under your Qt version, check "Qt Network Authorization"
#   4. Click "Next" and install
#
# Linux (Debian/Ubuntu):
#   sudo apt install libqt6networkauth6-dev
#
# Linux (Fedora):
#   sudo dnf install qt6-qtnetworkauth-devel
#
# macOS:
#   Install via Qt online installer (NetworkAuth included with Qt 6)
#   or with Homebrew: brew install qt@6
#
# After installation, restart Qt Creator and rebuild the project.

!qtHaveModule(networkauth) {
    error("Qt NetworkAuth module is required but not found. Please install it using the instructions above.")
}

QT += networkauth

CONFIG += c++17

# Disable deprecated APIs before Qt 6.0.0
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

# 原始碼檔案
SOURCES += \
    src/main.cpp \
    src/core/CalendarEvent.cpp \
    src/core/CalendarManager.cpp \
    src/adapters/GoogleCalendarAdapter.cpp \
    src/adapters/OutlookCalendarAdapter.cpp \
    src/storage/DatabaseManager.cpp \
    src/ui/MainWindow.cpp \
    src/ui/ChangeLogDialog.cpp

# 標頭檔案
HEADERS += \
    src/core/CalendarEvent.h \
    src/core/CalendarManager.h \
    src/adapters/CalendarAdapter.h \
    src/adapters/GoogleCalendarAdapter.h \
    src/adapters/OutlookCalendarAdapter.h \
    src/storage/DatabaseManager.h \
    src/ui/MainWindow.h \
    src/ui/ChangeLogDialog.h

# Include 目錄
INCLUDEPATH += $$PWD/src

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
