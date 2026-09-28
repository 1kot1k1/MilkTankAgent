#pragma once

#include "AppController.h"
#include "Logger.h"

#include <QHash>
#include <QMainWindow>
#include <QString>

class QListWidget;
class QListWidgetItem;
class QPlainTextEdit;
class QPushButton;
class QTabWidget;

class MainWindow : public QMainWindow
{
public:
    explicit MainWindow(
        AppController& controller,
        QWidget* parent = nullptr);

    ~MainWindow() override;

private:
    void handleDeviceEvent(
        const DeviceEvent& event);

    QListWidgetItem* findOrCreateItem(
        const DeviceEvent& event);

    void updateDeviceButtons();

    AppController& m_controller;

    EventBus::SubscriptionId m_subscriptionId = 0;
    Logger::SubscriptionId m_logSubscriptionId = 0;

    QTabWidget* m_tabWidget = nullptr;

    QListWidget* m_deviceList = nullptr;

    QPushButton* m_addDeviceButton = nullptr;
    QPushButton* m_editDeviceButton = nullptr;
    QPushButton* m_toggleDeviceButton = nullptr;
    QPushButton* m_removeDeviceButton = nullptr;
    QPushButton* m_settingsButton = nullptr;
    QPlainTextEdit* m_logView = nullptr;

    QHash<QString, QListWidgetItem*> m_deviceItems;
};