/****************************************************************************
** Meta object code from reading C++ file 'FormSelectSignals.h'
**
** Created: Tue Oct 29 12:33:47 2013
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "FormSelectSignals.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FormSelectSignals.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_FormSelectSignals[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      19,   18,   18,   18, 0x0a,
      71,   18,   18,   18, 0x0a,
     117,   18,   18,   18, 0x0a,
     173,   18,   18,   18, 0x0a,
     237,   18,   18,   18, 0x0a,
     291,   18,   18,   18, 0x0a,
     320,   18,   18,   18, 0x0a,
     343,   18,   18,   18, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_FormSelectSignals[] = {
    "FormSelectSignals\0\0"
    "tableWidgetFunctions_ItemClicked(QTableWidgetItem*)\0"
    "tableWidgetIRs_ItemClicked(QTableWidgetItem*)\0"
    "tableWidgetSignals_ItemDoubleClicked(QTableWidgetItem*)\0"
    "tableWidgetSelectedSignals_ItemDoubleClicked(QTableWidgetItem*)\0"
    "tableWidgetExtra_ItemDoubleClicked(QTableWidgetItem*)\0"
    "actionLoadAllOnChipSignals()\0"
    "pushButtonOK_clicked()\0"
    "pushButtonCancel_clicked()\0"
};

void FormSelectSignals::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        FormSelectSignals *_t = static_cast<FormSelectSignals *>(_o);
        switch (_id) {
        case 0: _t->tableWidgetFunctions_ItemClicked((*reinterpret_cast< QTableWidgetItem*(*)>(_a[1]))); break;
        case 1: _t->tableWidgetIRs_ItemClicked((*reinterpret_cast< QTableWidgetItem*(*)>(_a[1]))); break;
        case 2: _t->tableWidgetSignals_ItemDoubleClicked((*reinterpret_cast< QTableWidgetItem*(*)>(_a[1]))); break;
        case 3: _t->tableWidgetSelectedSignals_ItemDoubleClicked((*reinterpret_cast< QTableWidgetItem*(*)>(_a[1]))); break;
        case 4: _t->tableWidgetExtra_ItemDoubleClicked((*reinterpret_cast< QTableWidgetItem*(*)>(_a[1]))); break;
        case 5: _t->actionLoadAllOnChipSignals(); break;
        case 6: _t->pushButtonOK_clicked(); break;
        case 7: _t->pushButtonCancel_clicked(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData FormSelectSignals::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject FormSelectSignals::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_FormSelectSignals,
      qt_meta_data_FormSelectSignals, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &FormSelectSignals::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *FormSelectSignals::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *FormSelectSignals::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FormSelectSignals))
        return static_cast<void*>(const_cast< FormSelectSignals*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int FormSelectSignals::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
