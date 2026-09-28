#include "DeviceDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QString>
#include <QUuid>
#include <QVBoxLayout>

DeviceDialog::DeviceDialog(
    QWidget* parent)
    : QDialog(parent)
{
    createInterface();

    setWindowTitle(
        QStringLiteral(
            "\u0414\u043e\u0431\u0430\u0432\u043b\u0435\u043d\u0438\u0435 "
            "\u0443\u0441\u0442\u0440\u043e\u0439\u0441\u0442\u0432\u0430"));

    generateUuid();
}

DeviceDialog::DeviceDialog(
    const Device& device,
    QWidget* parent)
    : QDialog(parent)
{
    createInterface();

    setWindowTitle(
        QStringLiteral(
            "\u0418\u0437\u043c\u0435\u043d\u0435\u043d\u0438\u0435 "
            "\u0443\u0441\u0442\u0440\u043e\u0439\u0441\u0442\u0432\u0430"));

    fillFromDevice(
        device);
}

void DeviceDialog::createInterface()
{
    setModal(true);
    resize(480, 300);

    auto* mainLayout =
        new QVBoxLayout(this);

    auto* formLayout =
        new QFormLayout();

    m_nameEdit =
        new QLineEdit(this);

    m_nameEdit->setPlaceholderText(
        QStringLiteral(
            "\u041c\u043e\u043b\u043e\u0447\u043d\u044b\u0439 "
            "\u0442\u0430\u043d\u043a 1"));

    formLayout->addRow(
        QStringLiteral(
            "\u041d\u0430\u0437\u0432\u0430\u043d\u0438\u0435:"),
        m_nameEdit);

    m_serialNumberEdit =
        new QLineEdit(this);

    m_serialNumberEdit->setPlaceholderText(
        QStringLiteral(
            "\u041d\u0435\u043e\u0431\u044f\u0437\u0430\u0442\u0435\u043b\u044c\u043d\u043e"));

    m_serialNumberEdit->setMaxLength(
        8);

    formLayout->addRow(
        QStringLiteral(
            "\u0421\u0435\u0440\u0438\u0439\u043d\u044b\u0439 "
            "\u043d\u043e\u043c\u0435\u0440:"),
        m_serialNumberEdit);

    m_portEdit =
        new QLineEdit(this);

    m_portEdit->setText(
        "COM3");

    formLayout->addRow(
        QStringLiteral(
            "COM-\u043f\u043e\u0440\u0442:"),
        m_portEdit);

    m_baudRateCombo =
        new QComboBox(this);

    m_baudRateCombo->setEditable(
        true);

    m_baudRateCombo->addItems(
        {
            "9600",
            "19200",
            "38400",
            "57600",
            "115200"
        });

    m_baudRateCombo->setCurrentText(
        "115200");

    formLayout->addRow(
        QStringLiteral(
            "\u0421\u043a\u043e\u0440\u043e\u0441\u0442\u044c:"),
        m_baudRateCombo);

    m_addressSpinBox =
        new QSpinBox(this);

    m_addressSpinBox->setRange(
        1,
        247);

    m_addressSpinBox->setValue(
        1);

    formLayout->addRow(
        QStringLiteral(
            "Modbus-\u0430\u0434\u0440\u0435\u0441:"),
        m_addressSpinBox);

    auto* uuidLayout =
        new QHBoxLayout();

    m_uuidEdit =
        new QLineEdit(this);

    m_uuidEdit->setReadOnly(
        true);

    m_generateUuidButton =
        new QPushButton(
            QStringLiteral(
                "\u0421\u0433\u0435\u043d\u0435\u0440\u0438\u0440\u043e\u0432\u0430\u0442\u044c"),
            this);

    uuidLayout->addWidget(
        m_uuidEdit);

    uuidLayout->addWidget(
        m_generateUuidButton);

    formLayout->addRow(
        "UUID:",
        uuidLayout);

    m_enabledCheckBox =
        new QCheckBox(
            QStringLiteral(
                "\u0423\u0441\u0442\u0440\u043e\u0439\u0441\u0442\u0432\u043e "
                "\u0432\u043a\u043b\u044e\u0447\u0435\u043d\u043e"),
            this);

    m_enabledCheckBox->setChecked(
        true);

    formLayout->addRow(
        QString(),
        m_enabledCheckBox);

    mainLayout->addLayout(
        formLayout);

    auto* buttonBox =
        new QDialogButtonBox(
            QDialogButtonBox::Save |
            QDialogButtonBox::Cancel,
            this);

    buttonBox->button(
        QDialogButtonBox::Save)
        ->setText(
            QStringLiteral(
                "\u0421\u043e\u0445\u0440\u0430\u043d\u0438\u0442\u044c"));

    buttonBox->button(
        QDialogButtonBox::Cancel)
        ->setText(
            QStringLiteral(
                "\u041e\u0442\u043c\u0435\u043d\u0430"));

    mainLayout->addWidget(
        buttonBox);

    connect(
        m_generateUuidButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            generateUuid();
        });

    connect(
        buttonBox,
        &QDialogButtonBox::accepted,
        this,
        &DeviceDialog::accept);

    connect(
        buttonBox,
        &QDialogButtonBox::rejected,
        this,
        &DeviceDialog::reject);
}

void DeviceDialog::fillFromDevice(
    const Device& device)
{
    m_nameEdit->setText(
        QString::fromUtf8(
            device.name.c_str()));
    if (device.serialNumber.has_value())
    {
        m_serialNumberEdit->setText(
            QString::number(
                device.serialNumber.value()));
    }
    else
    {
        m_serialNumberEdit->clear();
    }
    m_portEdit->setText(
        QString::fromStdString(
            device.port));

    m_baudRateCombo->setCurrentText(
        QString::number(
            device.baudRate));

    m_addressSpinBox->setValue(
        device.address);

    m_uuidEdit->setText(
        QString::fromStdString(
            device.uuid));

    m_enabledCheckBox->setChecked(
        device.enabled);

    m_generateUuidButton->setEnabled(
        false);
}

void DeviceDialog::generateUuid()
{
    const QString uuid =
        QUuid::createUuid()
        .toString(
            QUuid::WithoutBraces);

    m_uuidEdit->setText(
        uuid);
}
Device DeviceDialog::device() const
{
    Device result;

    result.name =
        m_nameEdit->text()
        .trimmed()
        .toStdString();

    const QString serialText =
        m_serialNumberEdit->text()
        .trimmed();

    if (!serialText.isEmpty())
    {
        bool valid = false;

        const qulonglong value =
            serialText.toULongLong(
                &valid);

        if (valid &&
            value <= 16777215)
        {
            result.serialNumber =
                static_cast<std::uint32_t>(
                    value);
        }
    }

    result.port =
        m_portEdit->text()
        .trimmed()
        .toUpper()
        .toStdString();

    result.baudRate =
        m_baudRateCombo->currentText()
        .toInt();

    result.address =
        m_addressSpinBox->value();

    result.uuid =
        m_uuidEdit->text()
        .trimmed()
        .toStdString();

    result.enabled =
        m_enabledCheckBox->isChecked();

    return result;
}

void DeviceDialog::accept()
{
    if (m_nameEdit->text()
        .trimmed()
        .isEmpty())
    {
        QMessageBox::warning(
            this,
            QStringLiteral(
                "\u041e\u0448\u0438\u0431\u043a\u0430"),
            QStringLiteral(
                "\u0423\u043a\u0430\u0436\u0438\u0442\u0435 "
                "\u043d\u0430\u0437\u0432\u0430\u043d\u0438\u0435 "
                "\u0443\u0441\u0442\u0440\u043e\u0439\u0441\u0442\u0432\u0430."));

        m_nameEdit->setFocus();
        return;
    }

    if (m_portEdit->text()
        .trimmed()
        .isEmpty())
    {
        QMessageBox::warning(
            this,
            QStringLiteral(
                "\u041e\u0448\u0438\u0431\u043a\u0430"),
            QStringLiteral(
                "\u0423\u043a\u0430\u0436\u0438\u0442\u0435 "
                "COM-\u043f\u043e\u0440\u0442."));

        m_portEdit->setFocus();
        return;
    }

    bool baudRateValid = false;

    const int baudRate =
        m_baudRateCombo->currentText()
        .toInt(
            &baudRateValid);

    if (!baudRateValid ||
        baudRate <= 0)
    {
        QMessageBox::warning(
            this,
            QStringLiteral(
                "\u041e\u0448\u0438\u0431\u043a\u0430"),
            QStringLiteral(
                "\u0423\u043a\u0430\u0436\u0438\u0442\u0435 "
                "\u043a\u043e\u0440\u0440\u0435\u043a\u0442\u043d\u0443\u044e "
                "\u0441\u043a\u043e\u0440\u043e\u0441\u0442\u044c."));

        m_baudRateCombo->setFocus();
        return;
    }

    const QString serialText =
        m_serialNumberEdit->text()
        .trimmed();

    if (!serialText.isEmpty())
    {
        bool valid = false;

        const qulonglong value =
            serialText.toULongLong(
                &valid);

        if (!valid ||
            value > 16777215)
        {
            QMessageBox::warning(
                this,
                QStringLiteral(
                    "\u041e\u0448\u0438\u0431\u043a\u0430"),
                QStringLiteral(
                    "\u0421\u0435\u0440\u0438\u0439\u043d\u044b\u0439 "
                    "\u043d\u043e\u043c\u0435\u0440 "
                    "\u0434\u043e\u043b\u0436\u0435\u043d "
                    "\u0431\u044b\u0442\u044c "
                    "\u0447\u0438\u0441\u043b\u043e\u043c "
                    "\u043e\u0442 0 "
                    "\u0434\u043e 16777215."));

            m_serialNumberEdit->setFocus();
            m_serialNumberEdit->selectAll();

            return;
        }
    }

    if (m_uuidEdit->text()
        .trimmed()
        .isEmpty())
    {
        generateUuid();
    }

    QDialog::accept();
}