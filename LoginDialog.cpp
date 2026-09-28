#include "LoginDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

LoginDialog::LoginDialog(
    QWidget* parent)
    : QDialog(parent)
{
    createInterface();

    setWindowTitle(
        QStringLiteral(
            "\u0414\u043e\u0441\u0442\u0443\u043f "
            "\u043a \u043d\u0430\u0441\u0442\u0440\u043e\u0439\u043a\u0430\u043c"));
}

void LoginDialog::createInterface()
{
    setModal(true);
    resize(360, 130);

    auto* mainLayout =
        new QVBoxLayout(this);

    auto* formLayout =
        new QFormLayout();

    m_passwordEdit =
        new QLineEdit(this);

    m_passwordEdit->setEchoMode(
        QLineEdit::Password);

    formLayout->addRow(
        QStringLiteral(
            "\u041f\u0430\u0440\u043e\u043b\u044c:"),
        m_passwordEdit);

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
                "\u0412\u043e\u0439\u0442\u0438"));

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
        &LoginDialog::accept);

    connect(
        buttonBox,
        &QDialogButtonBox::rejected,
        this,
        &LoginDialog::reject);

    m_passwordEdit->setFocus();
}

QString LoginDialog::password() const
{
    return m_passwordEdit->text();
}

void LoginDialog::accept()
{
    if (m_passwordEdit->text()
        .isEmpty())
    {
        QMessageBox::warning(
            this,
            QStringLiteral(
                "\u041e\u0448\u0438\u0431\u043a\u0430"),
            QStringLiteral(
                "\u0423\u043a\u0430\u0436\u0438\u0442\u0435 "
                "\u043f\u0430\u0440\u043e\u043b\u044c."));

        m_passwordEdit->setFocus();

        return;
    }

    QDialog::accept();
}