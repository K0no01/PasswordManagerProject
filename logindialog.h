#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

class DatabaseManager;

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(DatabaseManager *db, QWidget *parent = nullptr);
    QByteArray masterKey() const; // возвращает ключ, если вход успешен

private slots:
    void onUnlock();

private:
    DatabaseManager *m_db;
    QLineEdit *m_passwordEdit;
    QLabel *m_statusLabel;
    QByteArray m_masterKey;
    bool m_firstRun;
};

#endif // LOGINDIALOG_H