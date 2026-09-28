#pragma once

#include "Device.h"

#include <QDialog>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QPushButton;
class QSpinBox;

class DeviceDialog : public QDialog
{
public:
    explicit DeviceDialog(
        QWidget* parent = nullptr);

    explicit DeviceDialog(
        const Device& device,
        QWidget* parent = nullptr);

    Device device() const;

protected:
    void accept() override;

private:
    void createInterface();
    void generateUuid();
    void fillFromDevice(
        const Device& device);

    QLineEdit* m_nameEdit = nullptr;
    QLineEdit* m_portEdit = nullptr;
    QComboBox* m_baudRateCombo = nullptr;
    QSpinBox* m_addressSpinBox = nullptr;
    QLineEdit* m_uuidEdit = nullptr;
    QPushButton* m_generateUuidButton = nullptr;
    QCheckBox* m_enabledCheckBox = nullptr;
    QLineEdit* m_serialNumberEdit = nullptr;

};