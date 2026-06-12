#include "databasemanager.h"
#include "Aes/aes.h"

#include <vector>
#include <cstring>
#include <iostream>

#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DatabaseManager::DatabaseManager(const QString &path)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen())
        m_db.close();
}

bool DatabaseManager::initialize()
{
    if (!m_db.open()) {
        qWarning() << "Failed to open DB:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery query(m_db);
    // Таблица для проверки мастер-пароля
    bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS master_key ("
        "id INTEGER PRIMARY KEY CHECK (id = 1), "
        "password_hash TEXT NOT NULL, "
        "salt TEXT NOT NULL)"
        );
    if (!ok) {
        qWarning() << "Failed to create master_key table:" << query.lastError().text();
        return false;
    }

    // Таблица аккаунтов
    ok = query.exec(
        "CREATE TABLE IF NOT EXISTS accounts ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL, "
        "url TEXT, "
        "username TEXT, "
        "encrypted_password BLOB, "
        "iv BLOB)"
        );
    if (!ok) {
        qWarning() << "Failed to create accounts table:" << query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::isMasterPasswordSet() const
{
    QSqlQuery query(m_db);
    query.exec("SELECT COUNT(*) FROM master_key WHERE id = 1");
    if (query.next())
        return query.value(0).toInt() > 0;
    return false;
}

bool DatabaseManager::setMasterPassword(const QString &password)
{
    // Генерируем случайную соль (16 байт)
    QByteArray salt(16, 0);
    auto *gen = QRandomGenerator::global();
    for (int i = 0; i < salt.size(); ++i)
        salt[i] = static_cast<char>(gen->bounded(256));
    QString saltHex = salt.toHex();

    // Хеш = SHA-256(password + salt)
    QByteArray hash = QCryptographicHash::hash(
        password.toUtf8() + salt, QCryptographicHash::Sha256);
    QString hashHex = hash.toHex();

    QSqlQuery query(m_db);
    query.prepare("INSERT OR REPLACE INTO master_key (id, password_hash, salt) VALUES (1, :hash, :salt)");
    query.bindValue(":hash", hashHex);
    query.bindValue(":salt", saltHex);
    if (!query.exec()) {
        qWarning() << "Failed to set master password:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::checkMasterPassword(const QString &password)
{
    QSqlQuery query(m_db);
    query.exec("SELECT password_hash, salt FROM master_key WHERE id = 1");
    if (!query.next())
        return false;
    QString storedHashHex = query.value(0).toString();
    QString saltHex = query.value(1).toString();
    QByteArray salt = QByteArray::fromHex(saltHex.toUtf8());
    QByteArray computedHash = QCryptographicHash::hash(
        password.toUtf8() + salt, QCryptographicHash::Sha256);
    return computedHash.toHex() == storedHashHex;
}

QByteArray DatabaseManager::deriveKey(const QString &password)
{
    const QByteArray staticSalt = "PasswordManagerSalt2024";
    QByteArray data = password.toUtf8() + staticSalt;
    return QCryptographicHash::hash(data, QCryptographicHash::Sha256);
}

QByteArray DatabaseManager::generateIv() const
{
    QByteArray iv(16, 0);
    auto *gen = QRandomGenerator::global();
    for (int i = 0; i < iv.size(); ++i)
        iv[i] = static_cast<char>(gen->bounded(256));
    return iv;
}

QByteArray DatabaseManager::encryptPassword(const QString &plain, const QByteArray &key, QByteArray &outIv) const
{
    std::cout << "encryptPassword: started, key size=" << key.size() << std::endl;

    outIv = generateIv();
    std::cout << "encryptPassword: iv generated, size=" << outIv.size() << std::endl;

    QByteArray plainData = plain.toUtf8();
    std::cout << "encryptPassword: plain size=" << plainData.size() << std::endl;

    // PKCS#7
    int padLen = 16 - (plainData.size() % 16);
    plainData.append(QByteArray(padLen, static_cast<char>(padLen)));
    std::cout << "encryptPassword: after padding, size=" << plainData.size() << std::endl;


    std::vector<uint8_t> buffer(plainData.size());
    std::memcpy(buffer.data(), plainData.constData(), plainData.size());

    struct AES_ctx ctx;
    std::cout << "encryptPassword: calling AES_init_ctx_iv..." << std::endl;
    AES_init_ctx_iv(&ctx,
                    reinterpret_cast<const uint8_t*>(key.constData()),
                    reinterpret_cast<const uint8_t*>(outIv.constData()));
    std::cout << "encryptPassword: init ok" << std::endl;

    std::cout << "encryptPassword: calling AES_CBC_encrypt_buffer..." << std::endl;
    AES_CBC_encrypt_buffer(&ctx, buffer.data(), buffer.size());
    std::cout << "encryptPassword: encrypt ok, cipher size=" << buffer.size() << std::endl;

    return QByteArray(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}

QString DatabaseManager::decryptPassword(const QByteArray &cipher, const QByteArray &iv, const QByteArray &key) const
{
    if (cipher.isEmpty() || iv.size() != 16)
        return {};

    QByteArray data = cipher;
    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx,
                    reinterpret_cast<const uint8_t*>(key.constData()),
                    reinterpret_cast<const uint8_t*>(iv.constData()));
    AES_CBC_decrypt_buffer(&ctx, reinterpret_cast<uint8_t*>(data.data()), data.size());


    if (data.isEmpty()) return {};
    int padLen = static_cast<unsigned char>(data.at(data.size() - 1));
    if (padLen > 0 && padLen <= 16)
        data.chop(padLen);

    return QString::fromUtf8(data);
}

bool DatabaseManager::addAccount(const Account &acc, const QString &password, const QByteArray &masterKey)
{
    std::cout << "addAccount: name=" << acc.name.toStdString()
    << ", password length=" << password.length() << std::endl;

    QByteArray iv;
    std::cout << "addAccount: encrypting..." << std::endl;
    QByteArray encPwd = encryptPassword(password, masterKey, iv);
    std::cout << "addAccount: encrypted size=" << encPwd.size() << std::endl;

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO accounts (name, url, username, encrypted_password, iv) "
                  "VALUES (:name, :url, :username, :enc, :iv)");
    query.bindValue(":name", acc.name);
    query.bindValue(":url", acc.url);
    query.bindValue(":username", acc.username);
    query.bindValue(":enc", encPwd);
    query.bindValue(":iv", iv);

    std::cout << "addAccount: executing query..." << std::endl;
    if (!query.exec()) {
        qWarning() << "Add account failed:" << query.lastError().text();
        std::cout << "addAccount: query failed" << std::endl;
        return false;
    }
    std::cout << "addAccount: success" << std::endl;
    return true;
}

bool DatabaseManager::updateAccount(const Account &acc, const QString &newPassword, const QByteArray &masterKey)
{
    QByteArray iv;
    QByteArray encPwd;

    if (!newPassword.isEmpty()) {
        // Шифруем новый пароль
        encPwd = encryptPassword(newPassword, masterKey, iv);
    } else {
        // Пароль не менялся — используем старый зашифрованный пароль из записи
        encPwd = acc.encryptedPassword;
        iv = acc.iv;
    }

    QSqlQuery query(m_db);
    query.prepare("UPDATE accounts SET name=:name, url=:url, username=:username, "
                  "encrypted_password=:enc, iv=:iv WHERE id=:id");
    query.bindValue(":name", acc.name);
    query.bindValue(":url", acc.url);
    query.bindValue(":username", acc.username);
    query.bindValue(":enc", encPwd);
    query.bindValue(":iv", iv);
    query.bindValue(":id", acc.id);

    if (!query.exec()) {
        qWarning() << "Update account failed:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::deleteAccount(int id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM accounts WHERE id=:id");
    query.bindValue(":id", id);
    return query.exec();
}

QVector<Account> DatabaseManager::getAllAccounts()
{
    QVector<Account> list;
    QSqlQuery query("SELECT id, name, url, username, encrypted_password, iv FROM accounts", m_db);
    while (query.next()) {
        Account acc;
        acc.id = query.value(0).toInt();
        acc.name = query.value(1).toString();
        acc.url = query.value(2).toString();
        acc.username = query.value(3).toString();
        acc.encryptedPassword = query.value(4).toByteArray();
        acc.iv = query.value(5).toByteArray();
        list.append(acc);
    }
    return list;
}

QString DatabaseManager::decryptAccountPassword(const Account &acc, const QByteArray &masterKey) const
{
    return decryptPassword(acc.encryptedPassword, acc.iv, masterKey);
}
