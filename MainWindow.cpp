#include "MainWindow.h"

#include <QBrush>
#include <QColor>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMetaObject>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QString>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>
#include "DeviceDialog.h"
#include <QDialog>
#include <string>
#include <QMessageBox>
#include "SettingsDialog.h"
#include "LoginDialog.h"

MainWindow::MainWindow(
    AppController& controller,
    QWidget* parent)
    : QMainWindow(parent),
    m_controller(controller)
{
    setWindowTitle(
        "Milk Tank Agent");

    resize(
        900,
        600);

    m_tabWidget =
        new QTabWidget(this);

    setCentralWidget(
        m_tabWidget);

    auto* devicesPage =
        new QWidget(m_tabWidget);

    auto* devicesLayout =
        new QVBoxLayout(devicesPage);

    m_deviceList =
        new QListWidget(devicesPage);

    m_deviceList->setSelectionMode(
        QAbstractItemView::SingleSelection);

    devicesLayout->addWidget(
        m_deviceList);

    auto* deviceButtonsLayout =
        new QHBoxLayout();

    m_addDeviceButton =
        new QPushButton(
            QStringLiteral(
                "\u0414\u043e\u0431\u0430\u0432\u0438\u0442\u044c"),
            devicesPage);

    m_editDeviceButton =
        new QPushButton(
            QStringLiteral(
                "\u0418\u0437\u043c\u0435\u043d\u0438\u0442\u044c"),
            devicesPage);

    m_toggleDeviceButton =
        new QPushButton(
            QStringLiteral(
                "\u0412\u043a\u043b\u044e\u0447\u0438\u0442\u044c / "
                "\u043e\u0442\u043a\u043b\u044e\u0447\u0438\u0442\u044c"),
            devicesPage);

    m_removeDeviceButton =
        new QPushButton(
            QStringLiteral(
                "\u0423\u0434\u0430\u043b\u0438\u0442\u044c"),
            devicesPage);
    m_settingsButton =
        new QPushButton(
            QStringLiteral(
                "\u041d\u0430\u0441\u0442\u0440\u043e\u0439\u043a\u0438"),
            devicesPage);

    deviceButtonsLayout->addWidget(
        m_addDeviceButton);

    deviceButtonsLayout->addWidget(
        m_editDeviceButton);

    deviceButtonsLayout->addWidget(
        m_toggleDeviceButton);

    deviceButtonsLayout->addWidget(
        m_removeDeviceButton);

    deviceButtonsLayout->addStretch();
    deviceButtonsLayout->addWidget(
        m_settingsButton);
    devicesLayout->addLayout(
        deviceButtonsLayout);

    m_tabWidget->addTab(
        devicesPage,
        QStringLiteral(
            "\u0423\u0441\u0442\u0440\u043e\u0439\u0441\u0442\u0432\u0430"));

    auto* logsPage =
        new QWidget(m_tabWidget);

    auto* logsLayout =
        new QVBoxLayout(logsPage);

    m_logView =
        new QPlainTextEdit(logsPage);

    m_logView->setReadOnly(
        true);

    m_logView->setPlaceholderText(
        QStringLiteral(
            "\u0417\u0434\u0435\u0441\u044c "
            "\u0431\u0443\u0434\u0443\u0442 "
            "\u043e\u0442\u043e\u0431\u0440\u0430\u0436\u0430\u0442\u044c\u0441\u044f "
            "\u043b\u043e\u0433\u0438 "
            "\u0430\u0433\u0435\u043d\u0442\u0430."));

    logsLayout->addWidget(
        m_logView);

    m_tabWidget->addTab(
        logsPage,
        QStringLiteral(
            "\u0416\u0443\u0440\u043d\u0430\u043b"));

    connect(
        m_deviceList,
        &QListWidget::currentItemChanged,
        this,
        [this](
            QListWidgetItem*,
            QListWidgetItem*)
        {
            updateDeviceButtons();
        });

    connect(
        m_addDeviceButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            DeviceDialog dialog(this);

            if (dialog.exec() != QDialog::Accepted)
            {
                return;
            }

            const Device device =
                dialog.device();

            if (!m_controller.addDevice(device))
            {
                QMessageBox::critical(
                    this,
                    QStringLiteral(
                        "\u041e\u0448\u0438\u0431\u043a\u0430"),
                    QStringLiteral(
                        "\u041d\u0435 \u0443\u0434\u0430\u043b\u043e\u0441\u044c "
                        "\u0434\u043e\u0431\u0430\u0432\u0438\u0442\u044c "
                        "\u0443\u0441\u0442\u0440\u043e\u0439\u0441\u0442\u0432\u043e.\n"
                        "\u041f\u0440\u043e\u0432\u0435\u0440\u044c\u0442\u0435 "
                        "\u0436\u0443\u0440\u043d\u0430\u043b."));

                return;
            }

            QMessageBox::information(
                this,
                QStringLiteral(
                    "\u0413\u043e\u0442\u043e\u0432\u043e"),
                QStringLiteral(
                    "\u0423\u0441\u0442\u0440\u043e\u0439\u0441\u0442\u0432\u043e "
                    "\u0434\u043e\u0431\u0430\u0432\u043b\u0435\u043d\u043e."));
        });



    connect(
        m_editDeviceButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            QListWidgetItem* selectedItem =
                m_deviceList->currentItem();

            if (!selectedItem)
            {
                return;
            }

            const QString uuid =
                selectedItem->data(
                    Qt::UserRole)
                .toString();

            Device existingDevice;

            if (!m_controller.getDevice(
                uuid.toStdString(),
                existingDevice))
            {
                QMessageBox::warning(
                    this,
                    QStringLiteral(
                        "\u041e\u0448\u0438\u0431\u043a\u0430"),
                    QStringLiteral(
                        "\u0423\u0441\u0442\u0440\u043e\u0439\u0441\u0442\u0432\u043e "
                        "\u043d\u0435 \u043d\u0430\u0439\u0434\u0435\u043d\u043e."));

                return;
            }

            DeviceDialog dialog(
                existingDevice,
                this);

            if (dialog.exec() !=
                QDialog::Accepted)
            {
                return;
            }

            const Device updatedDevice =
                dialog.device();

            if (!m_controller.updateDevice(
                updatedDevice))
            {
                QMessageBox::critical(
                    this,
                    QStringLiteral(
                        "\u041e\u0448\u0438\u0431\u043a\u0430"),
                    QStringLiteral(
                        "\u041d\u0435 \u0443\u0434\u0430\u043b\u043e\u0441\u044c "
                        "\u0441\u043e\u0445\u0440\u0430\u043d\u0438\u0442\u044c "
                        "\u0438\u0437\u043c\u0435\u043d\u0435\u043d\u0438\u044f.\n"
                        "\u041f\u0440\u043e\u0432\u0435\u0440\u044c\u0442\u0435 "
                        "\u0436\u0443\u0440\u043d\u0430\u043b."));

                return;
            }

            QMessageBox::information(
                this,
                QStringLiteral(
                    "\u0413\u043e\u0442\u043e\u0432\u043e"),
                QStringLiteral(
                    "\u0418\u0437\u043c\u0435\u043d\u0435\u043d\u0438\u044f "
                    "\u0441\u043e\u0445\u0440\u0430\u043d\u0435\u043d\u044b."));
        });

    connect(
        m_toggleDeviceButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            QListWidgetItem* selectedItem =
                m_deviceList->currentItem();

            if (!selectedItem)
            {
                return;
            }

            const QString uuid =
                selectedItem->data(
                    Qt::UserRole)
                .toString();

            Device device;

            if (!m_controller.getDevice(
                uuid.toStdString(),
                device))
            {
                QMessageBox::warning(
                    this,
                    QStringLiteral(
                        "\u041e\u0448\u0438\u0431\u043a\u0430"),
                    QStringLiteral(
                        "\u0423\u0441\u0442\u0440\u043e\u0439\u0441\u0442\u0432\u043e "
                        "\u043d\u0435 \u043d\u0430\u0439\u0434\u0435\u043d\u043e."));

                return;
            }

            const bool newEnabledState =
                !device.enabled;

            if (!m_controller.setDeviceEnabled(
                device.uuid,
                newEnabledState))
            {
                QMessageBox::critical(
                    this,
                    QStringLiteral(
                        "\u041e\u0448\u0438\u0431\u043a\u0430"),
                    QStringLiteral(
                        "\u041d\u0435 \u0443\u0434\u0430\u043b\u043e\u0441\u044c "
                        "\u0438\u0437\u043c\u0435\u043d\u0438\u0442\u044c "
                        "\u0441\u043e\u0441\u0442\u043e\u044f\u043d\u0438\u0435 "
                        "\u0443\u0441\u0442\u0440\u043e\u0439\u0441\u0442\u0432\u0430."));

                return;
            }

            QMessageBox::information(
                this,
                QStringLiteral(
                    "\u0413\u043e\u0442\u043e\u0432\u043e"),
                newEnabledState
                ? QStringLiteral(
                    "\u0423\u0441\u0442\u0440\u043e\u0439\u0441\u0442\u0432\u043e "
                    "\u0432\u043a\u043b\u044e\u0447\u0435\u043d\u043e.")
                : QStringLiteral(
                    "\u0423\u0441\u0442\u0440\u043e\u0439\u0441\u0442\u0432\u043e "
                    "\u043e\u0442\u043a\u043b\u044e\u0447\u0435\u043d\u043e."));
        });
    connect(
        m_removeDeviceButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            QListWidgetItem* selectedItem =
                m_deviceList->currentItem();

            if (!selectedItem)
            {
                return;
            }

            const QString uuid =
                selectedItem->data(
                    Qt::UserRole)
                .toString();

            Device device;

            if (!m_controller.getDevice(
                uuid.toStdString(),
                device))
            {
                QMessageBox::warning(
                    this,
                    QStringLiteral(
                        "\u041e\u0448\u0438\u0431\u043a\u0430"),
                    QStringLiteral(
                        "\u0423\u0441\u0442\u0440\u043e\u0439\u0441\u0442\u0432\u043e "
                        "\u043d\u0435 \u043d\u0430\u0439\u0434\u0435\u043d\u043e."));

                return;
            }

            const QMessageBox::StandardButton answer =
                QMessageBox::question(
                    this,
                    QStringLiteral(
                        "\u0423\u0434\u0430\u043b\u0435\u043d\u0438\u0435 "
                        "\u0443\u0441\u0442\u0440\u043e\u0439\u0441\u0442\u0432\u0430"),
                    QStringLiteral(
                        "\u0423\u0434\u0430\u043b\u0438\u0442\u044c "
                        "\u0443\u0441\u0442\u0440\u043e\u0439\u0441\u0442\u0432\u043e "
                        "\"%1\"?")
                    .arg(
                        QString::fromStdString(
                            device.name)),
                    QMessageBox::Yes |
                    QMessageBox::No,
                    QMessageBox::No);

            if (answer != QMessageBox::Yes)
            {
                return;
            }

            if (!m_controller.removeDevice(
                device.uuid))
            {
                QMessageBox::critical(
                    this,
                    QStringLiteral(
                        "\u041e\u0448\u0438\u0431\u043a\u0430"),
                    QStringLiteral(
                        "\u041d\u0435 \u0443\u0434\u0430\u043b\u043e\u0441\u044c "
                        "\u0443\u0434\u0430\u043b\u0438\u0442\u044c "
                        "\u0443\u0441\u0442\u0440\u043e\u0439\u0441\u0442\u0432\u043e.\n"
                        "\u041f\u0440\u043e\u0432\u0435\u0440\u044c\u0442\u0435 "
                        "\u0436\u0443\u0440\u043d\u0430\u043b."));

                return;
            }
        });
    connect(
        m_settingsButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            LoginDialog loginDialog(this);

            if (loginDialog.exec() != QDialog::Accepted)
            {
                return;
            }

            const QString password =
                loginDialog.password();

            if (!m_controller.verifyPassword(
                password.toStdString()))
            {
                QMessageBox::warning(
                    this,
                    QStringLiteral(
                        "\u041e\u0448\u0438\u0431\u043a\u0430"),
                    QStringLiteral(
                        "\u041d\u0435\u0432\u0435\u0440\u043d\u044b\u0439 "
                        "\u043f\u0430\u0440\u043e\u043b\u044c."));

                return;
            }

            const AppSettings currentSettings =
                m_controller.settings();

            SettingsDialog dialog(
                currentSettings,
                m_controller,
                password,
                this);

            if (dialog.exec() !=
                QDialog::Accepted)
            {
                return;
            }

            const AppSettings newSettings =
                dialog.settings();

            if (!m_controller.updateSettings(
                newSettings))
            {
                QMessageBox::critical(
                    this,
                    QStringLiteral(
                        "\u041e\u0448\u0438\u0431\u043a\u0430"),
                    QStringLiteral(
                        "\u041d\u0435 \u0443\u0434\u0430\u043b\u043e\u0441\u044c "
                        "\u0441\u043e\u0445\u0440\u0430\u043d\u0438\u0442\u044c "
                        "\u043d\u0430\u0441\u0442\u0440\u043e\u0439\u043a\u0438.\n"
                        "\u041f\u0440\u043e\u0432\u0435\u0440\u044c\u0442\u0435 "
                        "\u0436\u0443\u0440\u043d\u0430\u043b."));

                return;
            }
        });

    updateDeviceButtons();

    m_subscriptionId =
        m_controller.eventBus().subscribe(
            [this](
                const DeviceEvent& event)
            {
                QMetaObject::invokeMethod(
                    this,
                    [this, event]()
                    {
                        handleDeviceEvent(
                            event);
                    },
                    Qt::QueuedConnection);
            });

    m_logSubscriptionId =
        Logger::subscribe(
            [this](
                const std::string& line)
            {
                const QString text =
                    QString::fromUtf8(
                        line.c_str());

                QMetaObject::invokeMethod(
                    this,
                    [this, text]()
                    {
                        if (m_logView)
                        {
                            m_logView->appendPlainText(
                                text);
                        }
                    },
                    Qt::QueuedConnection);
            });
}

MainWindow::~MainWindow()
{
    if (m_subscriptionId != 0)
    {
        m_controller.eventBus().unsubscribe(
            m_subscriptionId);
    }

    if (m_logSubscriptionId != 0)
    {
        Logger::unsubscribe(
            m_logSubscriptionId);
    }
}

void MainWindow::updateDeviceButtons()
{
    const bool deviceSelected =
        m_deviceList != nullptr &&
        m_deviceList->currentItem() != nullptr;

    m_editDeviceButton->setEnabled(
        deviceSelected);

    m_toggleDeviceButton->setEnabled(
        deviceSelected);

    m_removeDeviceButton->setEnabled(
        deviceSelected);
}

QListWidgetItem* MainWindow::findOrCreateItem(
    const DeviceEvent& event)
{
    const QString uuid =
        QString::fromStdString(
            event.uuid);

    auto existing =
        m_deviceItems.find(
            uuid);

    if (existing != m_deviceItems.end())
    {
        return existing.value();
    }

    auto* item =
        new QListWidgetItem(
            m_deviceList);

    item->setData(
        Qt::UserRole,
        uuid);

    m_deviceItems.insert(
        uuid,
        item);

    return item;
}

void MainWindow::handleDeviceEvent(
    const DeviceEvent& event)
{
    const QString uuid =
        QString::fromStdString(
            event.uuid);

    if (event.type ==
        DeviceEventType::Removed)
    {
        QListWidgetItem* item =
            m_deviceItems.take(
                uuid);

        if (item)
        {
            delete item;
        }

        updateDeviceButtons();

        return;
    }

    QListWidgetItem* item =
        findOrCreateItem(
            event);

    QString text =
        QString::fromUtf8(
            event.name.c_str());

    switch (event.type)
    {
    case DeviceEventType::Online:
        text += " - ONLINE";

        item->setForeground(
            QBrush(
                QColor(
                    80,
                    200,
                    120)));
        break;

    case DeviceEventType::Offline:
        text += " - OFFLINE";

        item->setForeground(
            QBrush(
                QColor(
                    230,
                    80,
                    80)));
        break;

    case DeviceEventType::Disabled:
        text += " - DISABLED";

        item->setForeground(
            QBrush(
                QColor(
                    150,
                    150,
                    150)));
        break;

    case DeviceEventType::Overload:
        text += " - OVERLOAD";

        item->setForeground(
            QBrush(
                QColor(
                    230,
                    80,
                    80)));
        break;

    case DeviceEventType::Unstable:
        text += " - UNSTABLE";

        item->setForeground(
            QBrush(
                QColor(
                    230,
                    190,
                    70)));
        break;

    case DeviceEventType::WeightChanged:
    {
        double value =
            event.weight;

        QString unit =
            " g";

        int precision = 0;

        if (value >= 1000.0)
        {
            value /= 1000.0;
            unit = " kg";

            if (value < 10.0)
            {
                precision = 2;
            }
            else if (value < 100.0)
            {
                precision = 1;
            }
        }

        text +=
            QString(" - %1%2")
            .arg(
                value,
                0,
                'f',
                precision)
            .arg(
                unit);

        item->setForeground(
            QBrush(
                QColor(
                    80,
                    200,
                    120)));

        break;
    }

    case DeviceEventType::Added:
        text += " - ADDED";

        item->setForeground(
            QBrush(
                QColor(
                    100,
                    170,
                    230)));
        break;

    case DeviceEventType::Updated:
        text += " - UPDATED";

        item->setForeground(
            QBrush(
                QColor(
                    100,
                    170,
                    230)));
        break;

    case DeviceEventType::Removed:
        return;
    }

    item->setText(
        text);
}