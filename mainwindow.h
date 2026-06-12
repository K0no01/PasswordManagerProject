#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QByteArray>
#include <QStandardItemModel>

class DatabaseManager;
struct Account;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(DatabaseManager *db, const QByteArray &masterKey, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void onCopyPassword();
    void onOpenUrl();

private:
    Ui::MainWindow *ui;
    DatabaseManager *m_db;
    QByteArray m_masterKey;
    QStandardItemModel *m_model;

    void loadAccounts();
    Account selectedAccount() const;
};

#endif