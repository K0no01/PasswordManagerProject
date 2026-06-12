QT += widgets
QT += widgets sql

DEFINES += CBC=1 AES256=1

CONFIG += c++17
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Aes/aes.cpp \
    accountdialog.cpp \
    databasemanager.cpp \
    logindialog.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    Aes/aes.h \
    accountdialog.h \
    databasemanager.h \
    logindialog.h \
    mainwindow.h

FORMS += \
    mainwindow.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
