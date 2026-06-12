#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QString>
#include <QByteArray>
#include <QSqlDatabase>
#include <QVector>

struct Account {
    int id = -1;
    QString name;
    QString url;
    QString username;
    QByteArray encryptedPassword; // зашифрованный пароль
    QByteArray iv;                // вектор инициализации
};

class DatabaseManager
{
public:
    DatabaseManager(const QString &path);
    ~DatabaseManager();

    bool initialize();

    // Работа с мастер-паролем
    bool isMasterPasswordSet() const;
    bool setMasterPassword(const QString &password);
    bool checkMasterPassword(const QString &password);

    // CRUD для аккаунтов
    bool addAccount(const Account &acc, const QString &password, const QByteArray &masterKey);
    bool updateAccount(const Account &acc, const QString &newPassword, const QByteArray &masterKey);
    bool deleteAccount(int id);
    QVector<Account> getAllAccounts();

    // Расшифровка пароля конкретной записи
    QString decryptAccountPassword(const Account &acc, const QByteArray &masterKey) const;

    // Генерация ключа из пароля
    static QByteArray deriveKey(const QString &password);

private:
    QSqlDatabase m_db;

    QByteArray generateIv() const;
    QByteArray encryptPassword(const QString &plain, const QByteArray &key, QByteArray &outIv) const;
    QString decryptPassword(const QByteArray &cipher, const QByteArray &iv, const QByteArray &key) const;
};

#endif