#pragma once

#include <QDialog>
#include <QString>

class QLineEdit;

class ChangePasswordDialog : public QDialog
{
public:
    explicit ChangePasswordDialog(
        QWidget* parent = nullptr);

    QString newPassword() const;

protected:
    void accept() override;

private:
    void createInterface();

    QLineEdit* m_newPasswordEdit = nullptr;
    QLineEdit* m_confirmPasswordEdit = nullptr;
};