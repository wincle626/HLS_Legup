/********************************************************************************
** Form generated from reading UI file 'secondForm.ui'
**
** Created: Sat Oct 26 10:52:10 2013
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SECONDFORM_H
#define UI_SECONDFORM_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHeaderView>
#include <QtGui/QTableWidget>

QT_BEGIN_NAMESPACE

class Ui_secondForm
{
public:
    QDialogButtonBox *buttonBox;
    QTableWidget *tableWidgetSignlas;

    void setupUi(QDialog *secondForm)
    {
        if (secondForm->objectName().isEmpty())
            secondForm->setObjectName(QString::fromUtf8("secondForm"));
        secondForm->resize(428, 662);
        buttonBox = new QDialogButtonBox(secondForm);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setGeometry(QRect(30, 620, 381, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        tableWidgetSignlas = new QTableWidget(secondForm);
        if (tableWidgetSignlas->columnCount() < 3)
            tableWidgetSignlas->setColumnCount(3);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tableWidgetSignlas->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableWidgetSignlas->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tableWidgetSignlas->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        tableWidgetSignlas->setObjectName(QString::fromUtf8("tableWidgetSignlas"));
        tableWidgetSignlas->setGeometry(QRect(20, 10, 391, 601));

        retranslateUi(secondForm);
        QObject::connect(buttonBox, SIGNAL(accepted()), secondForm, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), secondForm, SLOT(reject()));

        QMetaObject::connectSlotsByName(secondForm);
    } // setupUi

    void retranslateUi(QDialog *secondForm)
    {
        secondForm->setWindowTitle(QApplication::translate("secondForm", "secondForm", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem = tableWidgetSignlas->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("secondForm", "Signal Name", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem1 = tableWidgetSignlas->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("secondForm", "Type", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem2 = tableWidgetSignlas->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("secondForm", "Creator", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class secondForm: public Ui_secondForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SECONDFORM_H
