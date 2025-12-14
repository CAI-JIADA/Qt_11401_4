# Qt 多平台行事曆整合工具

cmake_minimum_required(VERSION 3.16)
project(CalendarIntegration VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt 模組
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Network
    Sql
)

# 原始碼檔案
set(SOURCES
    src/main.cpp
    src/core/CalendarEvent.cpp
    src/core/CalendarManager.cpp
    src/adapters/CalDAVClient.cpp
    src/adapters/AppleCalendarAdapter.cpp
    src/adapters/GoogleCalendarAdapter.cpp
    src/adapters/OutlookCalendarAdapter.cpp
    src/storage/DatabaseManager.cpp
)

set(HEADERS
    src/core/CalendarEvent.h
    src/core/CalendarManager.h
    src/adapters/CalDAVClient.h
    src/adapters/CalendarAdapter.h
    src/adapters/AppleCalendarAdapter.h
    src/adapters/GoogleCalendarAdapter.h
    src/adapters/OutlookCalendarAdapter.h
    src/storage/DatabaseManager.h
)

# 執行檔
add_executable(CalendarIntegration
    ${SOURCES}
    ${HEADERS}
)

# 連結 Qt 函式庫
target_link_libraries(CalendarIntegration
    Qt6::Core
    Qt6::Network
    Qt6::Sql
)

# Include 目錄
target_include_directories(CalendarIntegration PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 安裝規則
install(TARGETS CalendarIntegration
    RUNTIME DESTINATION bin
)
