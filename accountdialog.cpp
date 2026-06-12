#include "accountdialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>

AccountDialog::AccountDialog(const Account &acc, QWidget *parent)
    : QDialog(parent), m_editId(acc.id)
{
    setWindowTitle(m_editId == -1 ? "Add Account" : "Edit Account");
    setMinimumWidth(350);

    auto *form = new QFormLayout;
    m_nameEdit = new QLineEdit(acc.name);
    m_urlEdit = new QLineEdit(acc.url);
    m_usernameEdit = new QLineEdit(acc.username);
    m_passwordEdit = new QLineEdit;
    m_passwordEdit->setEchoMode(QLineEdit::Password);

    if (m_editId != -1) {
        m_passwordEdit->setPlaceholderText("Leave blank to keep unchanged");
    } else {
        m_passwordEdit->setPlaceholderText("Enter password");
    }

    form->addRow("Name:", m_nameEdit);
    form->addRow("URL:", m_urlEdit);
    form->addRow("Username:", m_usernameEdit);
    form->addRow("Password:", m_passwordEdit);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttonBox);
}

Account AccountDialog::accountData() const
{
    Account acc;
    acc.id = m_editId;
    acc.name = m_nameEdit->text().trimmed();
    acc.url = m_urlEdit->text().trimmed();
    acc.username = m_usernameEdit->text().trimmed();
    // Временно кладём открытый пароль в encryptedPassword (будет зашифрован в БД)
    acc.encryptedPassword = m_passwordEdit->text().toUtf8();
    return acc;
}