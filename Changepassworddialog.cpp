#include "ChangePasswordDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

ChangePasswordDialog::ChangePasswordDialog(
    QWidget* parent)
    : QDialog(parent)
{
    createInterface();

    setWindowTitle(
        QStringLiteral(
            "\u0421\u043c\u0435\u043d\u0430 "
            "\u043f\u0430\u0440\u043e\u043b\u044f"));
}

void ChangePasswordDialog::createInterface()
{
    setModal(true);
    resize(360, 150);

    auto* mainLayout =
        new QVBoxLayout(this);

    auto* formLayout =
        new QFormLayout();

    m_newPasswordEdit =
        new QLineEdit(this);

    m_newPasswordEdit->setEchoMode(
        QLineEdit::Password);

    formLayout->addRow(
        QStringLiteral(
            "\u041d\u043e\u0432\u044b\u0439 "
            "\u043f\u0430\u0440\u043e\u043b\u044c:"),
        m_newPasswordEdit);

    m_confirmPasswordEdit =
        new QLineEdit(this);

    m_confirmPasswordEdit->setEchoMode(
        QLineEdit::Password);

    formLayout->addRow(
        QStringLiteral(
            "\u041f\u043e\u0432\u0442\u043e\u0440\u0438\u0442\u0435 "
            "\u043f\u0430\u0440\u043e\u043b\u044c:"),
        m_confirmPasswordEdit);

    mainLayout->addLayout(
        formLayout);

    auto* buttonBox =
        new QDialogButtonBox(
            QDialogButtonBox::Ok |
            QDialogButtonBox::Cancel,
            this);

    buttonBox->button(
        QDialogButtonBox::Ok)
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
        &ChangePasswordDialog::accept);

    connect(
        buttonBox,
        &QDialogButtonBox::rejected,
        this,
        &ChangePasswordDialog::reject);

    m_newPasswordEdit->setFocus();
}

QString ChangePasswordDialog::newPassword() const
{
    return m_newPasswordEdit->text();
}

void ChangePasswordDialog::accept()
{
    if (m_newPasswordEdit->text().isEmpty())
    {
        QMessageBox::warning(
            this,
            QStringLiteral(
                "\u041e\u0448\u0438\u0431\u043a\u0430"),
            QStringLiteral(
                "\u0423\u043a\u0430\u0436\u0438\u0442\u0435 "
                "\u043d\u043e\u0432\u044b\u0439 "
                "\u043f\u0430\u0440\u043e\u043b\u044c."));

        m_newPasswordEdit->setFocus();

        return;
    }

    if (m_newPasswordEdit->text() !=
        m_confirmPasswordEdit->text())
    {
        QMessageBox::warning(
            this,
            QStringLiteral(
                "\u041e\u0448\u0438\u0431\u043a\u0430"),
            QStringLiteral(
                "\u041f\u0430\u0440\u043e\u043b\u0438 "
                "\u043d\u0435 \u0441\u043e\u0432\u043f\u0430\u0434\u0430\u044e\u0442."));

        m_confirmPasswordEdit->setFocus();
        m_confirmPasswordEdit->selectAll();

        return;
    }

    QDialog::accept();
}