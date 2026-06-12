#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "databasemanager.h"
#include "accountdialog.h"

#include <QClipboard>
#include <QDesktopServices>
#include <QMessageBox>
#include <QUrl>
#include <QApplication>
#include <iostream>  

MainWindow::MainWindow(DatabaseManager *db, const QByteArray &masterKey, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_db(db)
    , m_masterKey(masterKey)
    , m_model(new QStandardItemModel(this))
{
    ui->setupUi(this);

    m_model->setHorizontalHeaderLabels({"Name", "URL", "Username"});
    ui->tableView->setModel(m_model);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    connect(ui->btnAdd, &QPushButton::clicked, this, &MainWindow::onAdd);
    connect(ui->btnEdit, &QPushButton::clicked, this, &MainWindow::onEdit);
    connect(ui->btnDelete, &QPushButton::clicked, this, &MainWindow::onDelete);
    connect(ui->btnCopy, &QPushButton::clicked, this, &MainWindow::onCopyPassword);
    connect(ui->btnOpen, &QPushButton::clicked, this, &MainWindow::onOpenUrl);

    loadAccounts();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadAccounts()
{
    m_model->removeRows(0, m_model->rowCount());
    QVector<Account> accounts = m_db->getAllAccounts();
    for (const auto &acc : accounts) {
        QList<QStandardItem*> row;
        row.append(new QStandardItem(acc.name));
        row.append(new QStandardItem(acc.url));
        row.append(new QStandardItem(acc.username));
        row[0]->setData(acc.id, Qt::UserRole);
        row[0]->setData(acc.encryptedPassword, Qt::UserRole + 1);
        row[0]->setData(acc.iv, Qt::UserRole + 2);
        m_model->appendRow(row);
    }
}

Account MainWindow::selectedAccount() const
{
    Account acc;
    QModelIndexList sel = ui->tableView->selectionModel()->selectedRows();
    if (sel.isEmpty()) return acc;
    int row = sel.first().row();
    QStandardItem *item = m_model->item(row, 0);
    acc.id = item->data(Qt::UserRole).toInt();
    acc.name = item->text();
    acc.url = m_model->item(row, 1)->text();
    acc.username = m_model->item(row, 2)->text();
    acc.encryptedPassword = item->data(Qt::UserRole + 1).toByteArray();
    acc.iv = item->data(Qt::UserRole + 2).toByteArray();
    return acc;
}

void MainWindow::onAdd()
{
    std::cout << "onAdd started" << std::endl;
    AccountDialog dlg;
    if (dlg.exec() == QDialog::Accepted) {
        std::cout << "Dialog accepted" << std::endl;
        Account acc = dlg.accountData();
        if (acc.name.isEmpty()) {
            QMessageBox::warning(this, "Warning", "Name cannot be empty.");
            return;
        }
        QString password = QString::fromUtf8(acc.encryptedPassword);
        std::cout << "Password to save: [" << password.toStdString() << "]" << std::endl;
        std::cout << "Calling addAccount..." << std::endl;
        bool result = m_db->addAccount(acc, password, m_masterKey);
        std::cout << "addAccount result: " << (result ? "true" : "false") << std::endl;
        if (result) {
            loadAccounts();
            std::cout << "Accounts reloaded" << std::endl;
        } else {
            QMessageBox::critical(this, "Error", "Failed to add account.");
            std::cout << "Error adding account" << std::endl;
        }
    }
    std::cout << "onAdd finished" << std::endl;
}

void MainWindow::onEdit()
{
    Account acc = selectedAccount();
    if (acc.id == -1) {
        QMessageBox::information(this, "Info", "Select an account first.");
        return;
    }
    AccountDialog dlg(acc);
    if (dlg.exec() == QDialog::Accepted) {
        Account updated = dlg.accountData();
        if (updated.name.isEmpty()) {
            QMessageBox::warning(this, "Warning", "Name cannot be empty.");
            return;
        }
        QString newPassword;
        if (!updated.encryptedPassword.isEmpty())
            newPassword = QString::fromUtf8(updated.encryptedPassword);
        if (m_db->updateAccount(updated, newPassword, m_masterKey))
            loadAccounts();
        else
            QMessageBox::critical(this, "Error", "Failed to update account.");
    }
}

void MainWindow::onDelete()
{
    Account acc = selectedAccount();
    if (acc.id == -1) {
        QMessageBox::information(this, "Info", "Select an account first.");
        return;
    }
    if (QMessageBox::question(this, "Confirm", "Delete this account?") == QMessageBox::Yes) {
        m_db->deleteAccount(acc.id);
        loadAccounts();
    }
}

void MainWindow::onCopyPassword()
{
    Account acc = selectedAccount();
    if (acc.id == -1) {
        QMessageBox::information(this, "Info", "Select an account first.");
        return;
    }
    QString password = m_db->decryptAccountPassword(acc, m_masterKey);
    QApplication::clipboard()->setText(password);
    ui->statusbar->showMessage("Password copied to clipboard", 3000);
}

void MainWindow::onOpenUrl()
{
    Account acc = selectedAccount();
    if (acc.id == -1) {
        QMessageBox::information(this, "Info", "Select an account first.");
        return;
    }
    QString url = acc.url;
    if (!url.startsWith("http://") && !url.startsWith("https://"))
        url = "https://" + url;
    QDesktopServices::openUrl(QUrl(url));
}
