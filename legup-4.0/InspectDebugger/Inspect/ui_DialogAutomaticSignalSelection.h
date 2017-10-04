/********************************************************************************
** Form generated from reading UI file 'DialogAutomaticSignalSelection.ui'
**
** Created: Mon Jul 14 11:10:31 2014
**      by: Qt User Interface Compiler version 4.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGAUTOMATICSIGNALSELECTION_H
#define UI_DIALOGAUTOMATICSIGNALSELECTION_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHeaderView>
#include <QtGui/QTableWidget>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DialogAutomaticSignalSelection
{
public:
    QVBoxLayout *verticalLayout;
    QTableWidget *tableWidgetSignalsToCycles;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *DialogAutomaticSignalSelection)
    {
        if (DialogAutomaticSignalSelection->objectName().isEmpty())
            DialogAutomaticSignalSelection->setObjectName(QString::fromUtf8("DialogAutomaticSignalSelection"));
        DialogAutomaticSignalSelection->resize(736, 585);
        verticalLayout = new QVBoxLayout(DialogAutomaticSignalSelection);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        tableWidgetSignalsToCycles = new QTableWidget(DialogAutomaticSignalSelection);
        tableWidgetSignalsToCycles->setObjectName(QString::fromUtf8("tableWidgetSignalsToCycles"));

        verticalLayout->addWidget(tableWidgetSignalsToCycles);

        buttonBox = new QDialogButtonBox(DialogAutomaticSignalSelection);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(DialogAutomaticSignalSelection);
        QObject::connect(buttonBox, SIGNAL(accepted()), DialogAutomaticSignalSelection, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), DialogAutomaticSignalSelection, SLOT(reject()));

        QMetaObject::connectSlotsByName(DialogAutomaticSignalSelection);
    } // setupUi

    void retranslateUi(QDialog *DialogAutomaticSignalSelection)
    {
        DialogAutomaticSignalSelection->setWindowTitle(QApplication::translate("DialogAutomaticSignalSelection", "DialogAutomaticSignalSelection", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DialogAutomaticSignalSelection: public Ui_DialogAutomaticSignalSelection {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGAUTOMATICSIGNALSELECTION_H
