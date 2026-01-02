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
    Gui
    Network
    NetworkAuth
    Sql
    Widgets
)

# 原始碼檔案
set(SOURCES
    src/main.cpp
    src/core/CalendarEvent.cpp
    src/core/CalendarManager.cpp
    src/adapters/GoogleCalendarAdapter.cpp
    src/adapters/OutlookCalendarAdapter.cpp
    src/storage/DatabaseManager.cpp
    src/ui/MainWindow.cpp
)

set(HEADERS
    src/core/CalendarEvent.h
    src/core/CalendarManager.h
    src/adapters/CalendarAdapter.h
    src/adapters/GoogleCalendarAdapter.h
    src/adapters/OutlookCalendarAdapter.h
    src/storage/DatabaseManager.h
    src/ui/MainWindow.h
)

# 執行檔
add_executable(CalendarIntegration
    ${SOURCES}
    ${HEADERS}
)

# 連結 Qt 函式庫
target_link_libraries(CalendarIntegration
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::NetworkAuth
    Qt6::Sql
    Qt6::Widgets
)

# Include 目錄
target_include_directories(CalendarIntegration PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Windows 平台自動部署 Qt 相依檔案
if(WIN32)
    # 確認 Qt6::Core 目標存在（應該由 find_package 保證）
    if(TARGET Qt6::Core)
        # 找到 windeployqt 工具 - 使用 Qt6::Core 更可靠
        get_target_property(_qt_core_location Qt6::Core IMPORTED_LOCATION_RELEASE)
        if(NOT _qt_core_location)
            get_target_property(_qt_core_location Qt6::Core IMPORTED_LOCATION)
        endif()
        
        if(_qt_core_location)
            get_filename_component(_qt_bin_dir "${_qt_core_location}" DIRECTORY)
            find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
            
            if(WINDEPLOYQT_EXECUTABLE)
                # 在建置後自動執行 windeployqt
                add_custom_command(TARGET CalendarIntegration POST_BUILD
                    COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                        --no-translations
                        --no-system-d3d-compiler
                        --no-opengl-sw
                        "$<TARGET_FILE:CalendarIntegration>"
                    COMMENT "執行 windeployqt 部署 Qt 相依檔案..."
                )
                message(STATUS "windeployqt 將在建置後自動執行")
            else()
                message(WARNING "找不到 windeployqt 工具，可能需要手動複製 Qt DLL 檔案")
            endif()
        else()
            message(WARNING "無法確定 Qt 安裝路徑，請手動執行 windeployqt 或複製 Qt DLL 檔案")
        endif()
    else()
        message(WARNING "Qt6::Core 目標不存在，無法自動部署 Qt 相依檔案")
    endif()
endif()

# 安裝規則
install(TARGETS CalendarIntegration
    RUNTIME DESTINATION bin
)
