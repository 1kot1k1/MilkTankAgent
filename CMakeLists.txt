cmake_minimum_required(VERSION 3.20)

project(MilkTankAgent VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH
    "C:/Qt/6.11.1/msvc2022_64"
)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

add_executable(MilkTankAgent WIN32

    main.cpp

    MainWindow.cpp
    MainWindow.h

    AppController.cpp
    AppController.h

    Logger.cpp
    Logger.h

    ConfigManager.cpp
    ConfigManager.h

    ModbusManager.cpp
    ModbusManager.h
    PollResult.h

    DataManager.cpp
    DataManager.h

    DatabaseManager.cpp
    DatabaseManager.h

    HttpClient.cpp
    HttpClient.h

    MeasurementQueue.cpp
    MeasurementQueue.h

    Device.h

    DeviceData.cpp
    DeviceData.h

    DeviceManager.cpp
    DeviceManager.h

    DeviceEvent.h

    EventBus.cpp
    EventBus.h

    LoggerSubscriber.cpp
    LoggerSubscriber.h

    DeviceDialog.cpp
    DeviceDialog.h

    AppSettings.h

    SettingsDialog.cpp
    SettingsDialog.h

    LoginDialog.cpp
    LoginDialog.h

    ChangePasswordDialog.cpp
    ChangePasswordDialog.h

    PasswordHasher.cpp
    PasswordHasher.h

    external/sqlite/sqlite3.c
)

target_include_directories(MilkTankAgent PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/sqlite
)

target_link_libraries(MilkTankAgent PRIVATE
    Qt6::Widgets
    bcrypt.lib
)

if(MSVC)
    target_compile_options(MilkTankAgent PRIVATE /utf-8)
endif()