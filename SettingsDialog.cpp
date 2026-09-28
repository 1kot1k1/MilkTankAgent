#include "SettingsDialog.h"
#include "AppController.h"
#include "ChangePasswordDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QString>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(
    const AppSettings& settings,
    AppController& controller,
    const QString& currentPassword,
    QWidget* parent)
    : QDialog(parent),
    m_controller(controller),
    m_currentPassword(currentPassword)
{
    setWindowTitle(
        QStringLiteral(
            "\u041d\u0430\u0441\u0442\u0440\u043e\u0439\u043a\u0438"));

    setModal(true);

    resize(
        560,
        420);

    auto* mainLayout =
        new QVBoxLayout(this);

    auto* formLayout =
        new QFormLayout();

    m_serverHostEdit =
        new QLineEdit(this);

    m_serverHostEdit->setText(
        QString::fromStdString(
            settings.serverHost));

    formLayout->addRow(
        QStringLiteral(
            "\u0410\u0434\u0440\u0435\u0441 \u0441\u0435\u0440\u0432\u0435\u0440\u0430:"),
        m_serverHostEdit);

    m_apiPathEdit =
        new QLineEdit(this);

    m_apiPathEdit->setText(
        QString::fromStdString(
            settings.apiPath));

    formLayout->addRow(
        QStringLiteral(
            "\u041f\u0443\u0442\u044c API:"),
        m_apiPathEdit);

    m_companyUuidEdit =
        new QLineEdit(this);

    m_companyUuidEdit->setText(
        QString::fromStdString(
            settings.companyUuid));

    formLayout->addRow(
        QStringLiteral(
            "UUID \u043a\u043e\u043c\u043f\u0430\u043d\u0438\u0438:"),
        m_companyUuidEdit);

    m_farmUuidEdit =
        new QLineEdit(this);

    m_farmUuidEdit->setText(
        QString::fromStdString(
            settings.farmUuid));

    formLayout->addRow(
        QStringLiteral(
            "UUID \u0444\u0435\u0440\u043c\u044b:"),
        m_farmUuidEdit);

    m_herdUuidEdit =
        new QLineEdit(this);

    m_herdUuidEdit->setText(
        QString::fromStdString(
            settings.herdUuid));

    formLayout->addRow(
        QStringLiteral(
            "UUID \u0441\u0442\u0430\u0434\u0430:"),
        m_herdUuidEdit);

    m_pollIntervalSpinBox =
        new QSpinBox(this);

    m_pollIntervalSpinBox->setRange(
        1,
        3600);

    m_pollIntervalSpinBox->setSuffix(
        QStringLiteral(
            " \u0441\u0435\u043a."));

    m_pollIntervalSpinBox->setValue(
        settings.pollInterval);

    formLayout->addRow(
        QStringLiteral(
            "\u0418\u043d\u0442\u0435\u0440\u0432\u0430\u043b "
            "\u043e\u043f\u0440\u043e\u0441\u0430:"),
        m_pollIntervalSpinBox);

    m_sendIntervalSpinBox =
        new QSpinBox(this);

    m_sendIntervalSpinBox->setRange(
        1,
        86400);

    m_sendIntervalSpinBox->setSuffix(
        QStringLiteral(
            " \u0441\u0435\u043a."));

    m_sendIntervalSpinBox->setValue(
        settings.sendInterval);

    formLayout->addRow(
        QStringLiteral(
            "\u0418\u043d\u0442\u0435\u0440\u0432\u0430\u043b "
            "\u043e\u0442\u043f\u0440\u0430\u0432\u043a\u0438:"),
        m_sendIntervalSpinBox);

    m_retentionDaysSpinBox =
        new QSpinBox(this);

    m_retentionDaysSpinBox->setRange(
        1,
        3650);

    m_retentionDaysSpinBox->setSuffix(
        QStringLiteral(
            " \u0434\u043d."));

    m_retentionDaysSpinBox->setValue(
        settings.retentionDays);

    m_retentionDaysSpinBox->setToolTip(
        QStringLiteral(
            "\u0421\u043a\u043e\u043b\u044c\u043a\u043e \u0434\u043d\u0435\u0439 \u0445\u0440\u0430\u043d\u0438\u0442\u044c "
            "\u0432 \u043b\u043e\u043a\u0430\u043b\u044c\u043d\u043e\u0439 \u0431\u0430\u0437\u0435 "
            "\u043d\u0435\u043e\u0442\u043f\u0440\u0430\u0432\u043b\u0435\u043d\u043d\u044b\u0435 "
            "\u0434\u0430\u043d\u043d\u044b\u0435 (\u043f\u0440\u0438 \u0434\u043b\u0438\u0442\u0435\u043b\u044c\u043d\u043e\u043c "
            "\u043e\u0442\u0441\u0443\u0442\u0441\u0442\u0432\u0438\u0438 \u0441\u0432\u044f\u0437\u0438)."));

    formLayout->addRow(
        QStringLiteral(
            "\u0425\u0440\u0430\u043d\u0438\u0442\u044c "
            "\u043d\u0435\u043e\u0442\u043f\u0440\u0430\u0432\u043b\u0435\u043d\u043d\u044b\u0435 "
            "\u0434\u0430\u043d\u043d\u044b\u0435:"),
        m_retentionDaysSpinBox);

    m_verifyTlsCheckBox =
        new QCheckBox(
            QStringLiteral(
                "\u041f\u0440\u043e\u0432\u0435\u0440\u044f\u0442\u044c "
                "TLS-\u0441\u0435\u0440\u0442\u0438\u0444\u0438\u043a\u0430\u0442 "
                "\u0441\u0435\u0440\u0432\u0435\u0440\u0430"),
            this);

    m_verifyTlsCheckBox->setChecked(
        settings.verifyTlsCertificate);

    formLayout->addRow(
        QString(),
        m_verifyTlsCheckBox);

    mainLayout->addLayout(
        formLayout);

    auto* passwordLayout =
        new QHBoxLayout();

    m_changePasswordButton =
        new QPushButton(
            QStringLiteral(
                "\u0421\u043c\u0435\u043d\u0438\u0442\u044c \u043f\u0430\u0440\u043e\u043b\u044c"),
            this);

    passwordLayout->addStretch();
    passwordLayout->addWidget(
        m_changePasswordButton);

    mainLayout->addLayout(
        passwordLayout);

    auto* databaseLayout =
        new QHBoxLayout();

    m_databaseSizeLabel =
        new QLabel(this);

    m_clearDatabaseButton =
        new QPushButton(
            QStringLiteral(
                "\u041e\u0447\u0438\u0441\u0442\u0438\u0442\u044c "
                "\u0441\u0435\u0439\u0447\u0430\u0441"),
            this);

    databaseLayout->addWidget(
        m_databaseSizeLabel);

    databaseLayout->addStretch();
    databaseLayout->addWidget(
        m_clearDatabaseButton);

    mainLayout->addLayout(
        databaseLayout);

    updateDatabaseSizeLabel();

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
        buttonBox,
        &QDialogButtonBox::accepted,
        this,
        &SettingsDialog::accept);

    connect(
        buttonBox,
        &QDialogButtonBox::rejected,
        this,
        &SettingsDialog::reject);

    connect(
        m_changePasswordButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            changePassword();
        });

    connect(
        m_clearDatabaseButton,
        &QPushButton::clicked,
        this,
        [this]()
        {
            clearDatabase();
        });
}

AppSettings SettingsDialog::settings() const
{
    AppSettings result;

    result.serverHost =
        m_serverHostEdit->text()
        .trimmed()
        .toStdString();

    result.apiPath =
        m_apiPathEdit->text()
        .trimmed()
        .toStdString();

    result.companyUuid =
        m_companyUuidEdit->text()
        .trimmed()
        .toStdString();

    result.farmUuid =
        m_farmUuidEdit->text()
        .trimmed()
        .toStdString();

    result.herdUuid =
        m_herdUuidEdit->text()
        .trimmed()
        .toStdString();

    result.pollInterval =
        m_pollIntervalSpinBox->value();

    result.sendInterval =
        m_sendIntervalSpinBox->value();

    result.retentionDays =
        m_retentionDaysSpinBox->value();

    result.verifyTlsCertificate =
        m_verifyTlsCheckBox->isChecked();

    return result;
}

void SettingsDialog::accept()
{
    if (m_serverHostEdit->text()
        .trimmed()
        .isEmpty())
    {
        QMessageBox::warning(
            this,
            QStringLiteral(
                "\u041e\u0448\u0438\u0431\u043a\u0430"),
            QStringLiteral(
                "\u0423\u043a\u0430\u0436\u0438\u0442\u0435 "
                "\u0430\u0434\u0440\u0435\u0441 "
                "\u0441\u0435\u0440\u0432\u0435\u0440\u0430."));

        m_serverHostEdit->setFocus();

        return;
    }

    if (m_apiPathEdit->text()
        .trimmed()
        .isEmpty())
    {
        QMessageBox::warning(
            this,
            QStringLiteral(
                "\u041e\u0448\u0438\u0431\u043a\u0430"),
            QStringLiteral(
                "\u0423\u043a\u0430\u0436\u0438\u0442\u0435 "
                "\u043f\u0443\u0442\u044c API."));

        m_apiPathEdit->setFocus();

        return;
    }

    QDialog::accept();
}

void SettingsDialog::changePassword()
{
    ChangePasswordDialog dialog(this);

    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    const QString newPassword =
        dialog.newPassword();

    if (!m_controller.changePassword(
        m_currentPassword.toStdString(),
        newPassword.toStdString()))
    {
        QMessageBox::critical(
            this,
            QStringLiteral(
                "\u041e\u0448\u0438\u0431\u043a\u0430"),
            QStringLiteral(
                "\u041d\u0435 \u0443\u0434\u0430\u043b\u043e\u0441\u044c "
                "\u0441\u043c\u0435\u043d\u0438\u0442\u044c "
                "\u043f\u0430\u0440\u043e\u043b\u044c."));

        return;
    }

    m_currentPassword =
        newPassword;

    QMessageBox::information(
        this,
        QStringLiteral(
            "\u0413\u043e\u0442\u043e\u0432\u043e"),
        QStringLiteral(
            "\u041f\u0430\u0440\u043e\u043b\u044c \u0438\u0437\u043c\u0435\u043d\u0451\u043d."));
}

void SettingsDialog::updateDatabaseSizeLabel()
{
    const std::uintmax_t bytes =
        m_controller.databaseSizeBytes();

    QString text;

    if (bytes < 1024)
    {
        text =
            QStringLiteral(
                "\u0420\u0430\u0437\u043c\u0435\u0440 \u043b\u043e\u043a\u0430\u043b\u044c\u043d\u043e\u0439 "
                "\u0411\u0414: %1 \u0431\u0430\u0439\u0442")
            .arg(bytes);
    }
    else if (bytes < 1024ull * 1024)
    {
        text =
            QStringLiteral(
                "\u0420\u0430\u0437\u043c\u0435\u0440 \u043b\u043e\u043a\u0430\u043b\u044c\u043d\u043e\u0439 "
                "\u0411\u0414: %1 \u041a\u0411")
            .arg(
                QString::number(
                    bytes / 1024.0,
                    'f',
                    1));
    }
    else
    {
        text =
            QStringLiteral(
                "\u0420\u0430\u0437\u043c\u0435\u0440 \u043b\u043e\u043a\u0430\u043b\u044c\u043d\u043e\u0439 "
                "\u0411\u0414: %1 \u041c\u0411")
            .arg(
                QString::number(
                    bytes / (1024.0 * 1024.0),
                    'f',
                    1));
    }

    m_databaseSizeLabel->setText(text);
}

void SettingsDialog::clearDatabase()
{
    const auto answer =
        QMessageBox::question(
            this,
            QStringLiteral(
                "\u041f\u043e\u0434\u0442\u0432\u0435\u0440\u0436\u0434\u0435\u043d\u0438\u0435"),
            QStringLiteral(
                "\u041e\u0447\u0438\u0441\u0442\u0438\u0442\u044c \u043b\u043e\u043a\u0430\u043b\u044c\u043d\u0443\u044e "
                "\u0431\u0430\u0437\u0443?\n\n"
                "\u0412\u0441\u0435 \u043d\u0435\u043e\u0442\u043f\u0440\u0430\u0432\u043b\u0435\u043d\u043d\u044b\u0435 "
                "\u0434\u0430\u043d\u043d\u044b\u0435 (\u043d\u0430\u043a\u043e\u043f\u0438\u0432\u0448\u0438\u0435\u0441\u044f "
                "\u0438\u0437-\u0437\u0430 \u043e\u0442\u0441\u0443\u0442\u0441\u0442\u0432\u0438\u044f \u0441\u0432\u044f\u0437\u0438) "
                "\u0431\u0443\u0434\u0443\u0442 \u0431\u0435\u0437\u0432\u043e\u0437\u0432\u0440\u0430\u0442\u043d\u043e "
                "\u043f\u043e\u0442\u0435\u0440\u044f\u043d\u044b \u0438 \u043d\u0435 \u043f\u043e\u043f\u0430\u0434\u0443\u0442 "
                "\u043d\u0430 \u0441\u0435\u0440\u0432\u0435\u0440."),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

    if (answer != QMessageBox::Yes)
    {
        return;
    }

    if (!m_controller.clearDatabase())
    {
        QMessageBox::critical(
            this,
            QStringLiteral(
                "\u041e\u0448\u0438\u0431\u043a\u0430"),
            QStringLiteral(
                "\u041d\u0435 \u0443\u0434\u0430\u043b\u043e\u0441\u044c "
                "\u043e\u0447\u0438\u0441\u0442\u0438\u0442\u044c \u0431\u0430\u0437\u0443."));

        return;
    }

    updateDatabaseSizeLabel();

    QMessageBox::information(
        this,
        QStringLiteral(
            "\u0413\u043e\u0442\u043e\u0432\u043e"),
        QStringLiteral(
            "\u041b\u043e\u043a\u0430\u043b\u044c\u043d\u0430\u044f \u0431\u0430\u0437\u0430 "
            "\u043e\u0447\u0438\u0449\u0435\u043d\u0430."));
}