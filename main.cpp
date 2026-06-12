#include "mainwindow.h"
#include "databasemanager.h"
#include "logindialog.h"
#include <QApplication>
#include <QStandardPaths>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Путь в AppData/Roaming/PasswordManager/passwords.db
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath);
    QString dbPath = dataPath + "/passwords.db";

    DatabaseManager db(dbPath);
    if (!db.initialize()) {
        qWarning("Failed to initialize database");
        return 1;
    }

    LoginDialog login(&db);
    if (login.exec() != QDialog::Accepted)
        return 0;

    QByteArray masterKey = login.masterKey();
    MainWindow w(&db, masterKey);
    w.show();

    return a.exec();
}