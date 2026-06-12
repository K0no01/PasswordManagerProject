#include "logindialog.h"
#include "databasemanager.h"

#include <QVBoxLayout>
#include <QMessageBox>
#include <QSqlQuery>

LoginDialog::LoginDialog(DatabaseManager *db, QWidget *parent)
    : QDialog(parent), m_db(db)
{
    setWindowTitle("Password Manager");
    setMinimumSize(350, 150);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *label = new QLabel(this);
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Enter master password...");
    m_statusLabel = new QLabel(this);

    QPushButton *unlockBtn = new QPushButton("Unlock", this);

    layout->addWidget(label);
    layout->addWidget(m_passwordEdit);
    layout->addWidget(unlockBtn);
    layout->addWidget(m_statusLabel);

    // Определяем, первый ли это запуск
    m_firstRun = !m_db->isMasterPasswordSet();

    if (m_firstRun) {
        label->setText("First run: create a master password.");
        setWindowTitle("Create Master Password");
        unlockBtn->setText("Create");
    } else {
        label->setText("Enter master password:");
    }

    connect(unlockBtn, &QPushButton::clicked, this, &LoginDialog::onUnlock);
}

QByteArray LoginDialog::masterKey() const
{
    return m_masterKey;
}

void LoginDialog::onUnlock()
{
    QString password = m_passwordEdit->text();
    if (password.isEmpty()) {
        m_statusLabel->setText("Password cannot be empty.");
        return;
    }

    if (m_firstRun) {
        // Создаём мастер-пароль
        if (m_db->setMasterPassword(password)) {
            m_masterKey = DatabaseManager::deriveKey(password);
            accept(); // закрываем диалог с успехом
        } else {
            m_statusLabel->setText("Failed to create master password.");
        }
    } else {
        // Проверяем существующий
        if (m_db->checkMasterPassword(password)) {
            m_masterKey = DatabaseManager::deriveKey(password);
            accept();
        } else {
            m_statusLabel->setText("Wrong password. Try again.");
        }
    }
}