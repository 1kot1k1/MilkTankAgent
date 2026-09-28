#pragma once

#include <QDialog>
#include <QString>

class QLineEdit;

class LoginDialog : public QDialog
{
public:
    explicit LoginDialog(
        QWidget* parent = nullptr);

    QString password() const;

protected:
    void accept() override;

private:
    void createInterface();

    QLineEdit* m_passwordEdit = nullptr;
};