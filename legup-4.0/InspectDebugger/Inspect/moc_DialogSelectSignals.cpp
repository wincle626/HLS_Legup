/****************************************************************************
** Meta object code from reading C++ file 'DialogSelectSignals.h'
**
** Created: Mon Jul 14 11:11:32 2014
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "DialogSelectSignals.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DialogSelectSignals.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DialogSelectSignals[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      21,   20,   20,   20, 0x0a,
      73,   20,   20,   20, 0x0a,
     119,   20,   20,   20, 0x0a,
     175,   20,   20,   20, 0x0a,
     239,   20,   20,   20, 0x0a,
     293,   20,   20,   20, 0x0a,
     322,   20,   20,   20, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_DialogSelectSignals[] = {
    "DialogSelectSignals\0\0"
    "tableWidgetFunctions_ItemClicked(QTableWidgetItem*)\0"
    "tableWidgetIRs_ItemClicked(QTableWidgetItem*)\0"
    "tableWidgetSignals_ItemDoubleClicked(QTableWidgetItem*)\0"
    "tableWidgetSelectedSignals_ItemDoubleClicked(QTableWidgetItem*)\0"
    "tableWidgetExtra_ItemDoubleClicked(QTableWidgetItem*)\0"
    "actionLoadAllOnChipSignals()\0"
    "pushButtonLoadSignals_clicked()\0"
};

const QMetaObject DialogSelectSignals::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_DialogSelectSignals,
      qt_meta_data_DialogSelectSignals, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DialogSelectSignals::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DialogSelectSignals::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DialogSelectSignals::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DialogSelectSignals))
        return static_cast<void*>(const_cast< DialogSelectSignals*>(this));
    return QDialog::qt_metacast(_clname);
}

int DialogSelectSignals::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: tableWidgetFunctions_ItemClicked((*reinterpret_cast< QTableWidgetItem*(*)>(_a[1]))); break;
        case 1: tableWidgetIRs_ItemClicked((*reinterpret_cast< QTableWidgetItem*(*)>(_a[1]))); break;
        case 2: tableWidgetSignals_ItemDoubleClicked((*reinterpret_cast< QTableWidgetItem*(*)>(_a[1]))); break;
        case 3: tableWidgetSelectedSignals_ItemDoubleClicked((*reinterpret_cast< QTableWidgetItem*(*)>(_a[1]))); break;
        case 4: tableWidgetExtra_ItemDoubleClicked((*reinterpret_cast< QTableWidgetItem*(*)>(_a[1]))); break;
        case 5: actionLoadAllOnChipSignals(); break;
        case 6: pushButtonLoadSignals_clicked(); break;
        default: ;
        }
        _id -= 7;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
