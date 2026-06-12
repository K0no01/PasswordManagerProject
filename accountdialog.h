#ifndef ACCOUNTDIALOG_H
#define ACCOUNTDIALOG_H

#include <QDialog>                  // <--- меняем QWidget на QDialog
#include <QLineEdit>
#include "databasemanager.h"        // нужен для структуры Account

class AccountDialog : public QDialog   // <--- наследуемся от QDialog
{
    Q_OBJECT
public:
    // Если acc.id == -1 — режим добавления, иначе редактирование
    explicit AccountDialog(const Account &acc = Account(), QWidget *parent = nullptr);
    Account accountData() const;   // возвращает заполненную запись

private:
    QLineEdit *m_nameEdit;
    QLineEdit *m_urlEdit;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    int m_editId;
};

#endif