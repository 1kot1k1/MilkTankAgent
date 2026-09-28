#pragma once

#include "AppSettings.h"

#include <QDialog>
#include <QString>

class QLineEdit;
class QSpinBox;
class QPushButton;
class QCheckBox;
class QLabel;
class AppController;

class SettingsDialog : public QDialog
{
public:
    explicit SettingsDialog(
        const AppSettings& settings,
        AppController& controller,
        const QString& currentPassword,
        QWidget* parent = nullptr);

    AppSettings settings() const;

protected:
    void accept() override;

private:
    void createInterface();
    void changePassword();
    void clearDatabase();
    void updateDatabaseSizeLabel();

    AppController& m_controller;
    QString m_currentPassword;

    QLineEdit* m_serverHostEdit = nullptr;
    QLineEdit* m_companyUuidEdit = nullptr;
    QLineEdit* m_farmUuidEdit = nullptr;
    QLineEdit* m_herdUuidEdit = nullptr;
    QLineEdit* m_apiPathEdit = nullptr;
    QCheckBox* m_verifyTlsCheckBox = nullptr;

    QSpinBox* m_pollIntervalSpinBox = nullptr;
    QSpinBox* m_sendIntervalSpinBox = nullptr;
    QSpinBox* m_retentionDaysSpinBox = nullptr;

    QPushButton* m_changePasswordButton = nullptr;
    QPushButton* m_clearDatabaseButton = nullptr;
    QLabel* m_databaseSizeLabel = nullptr;
};