# Qt 多平台行事曆整合工具
QT += core gui network networkauth sql widgets

CONFIG += c++17

# Disable deprecated APIs before Qt 6.0.0
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

# 原始碼檔案
SOURCES += \
    src/main.cpp \
    src/core/CalendarEvent.cpp \
    src/core/CalendarManager.cpp \
    src/adapters/CalDAVClient.cpp \
    src/adapters/AppleCalendarAdapter.cpp \
    src/adapters/GoogleCalendarAdapter.cpp \
    src/adapters/OutlookCalendarAdapter.cpp \
    src/storage/DatabaseManager.cpp \
    src/ui/MainWindow.cpp

# 標頭檔案
HEADERS += \
    src/core/CalendarEvent.h \
    src/core/CalendarManager.h \
    src/adapters/CalDAVClient.h \
    src/adapters/CalendarAdapter.h \
    src/adapters/AppleCalendarAdapter.h \
    src/adapters/GoogleCalendarAdapter.h \
    src/adapters/OutlookCalendarAdapter.h \
    src/storage/DatabaseManager.h \
    src/ui/MainWindow.h

# Include 目錄
INCLUDEPATH += $$PWD/src

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
