#ifndef ACCOUNTDIALOG_H
#define ACCOUNTDIALOG_H

#include <QDialog>                 
#include <QLineEdit>
#include "databasemanager.h"      

class AccountDialog : public QDialog  
    Q_OBJECT
public:
    explicit AccountDialog(const Account &acc = Account(), QWidget *parent = nullptr);
    Account accountData() const;  
private:
    QLineEdit *m_nameEdit;
    QLineEdit *m_urlEdit;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    int m_editId;
};

#endif
