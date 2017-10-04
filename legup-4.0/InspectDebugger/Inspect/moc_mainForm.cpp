/****************************************************************************
** Meta object code from reading C++ file 'mainForm.h'
**
** Created: Tue Oct 29 10:58:01 2013
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mainForm.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainForm.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_mainForm[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x0a,
      41,    9,    9,    9, 0x0a,
      76,    9,    9,    9, 0x0a,
     100,    9,    9,    9, 0x0a,
     128,    9,    9,    9, 0x0a,
     168,  163,    9,    9, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_mainForm[] = {
    "mainForm\0\0pushButtonLoadDesign_clicked()\0"
    "pushButtonOpenConnection_clicked()\0"
    "pushButtonRun_clicked()\0"
    "pushButtonExamine_clicked()\0"
    "pushButtonSingleStepping_clicked()\0"
    "item\0treeWidgetIR_itemClicked(QTreeWidgetItem*)\0"
};

void mainForm::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        mainForm *_t = static_cast<mainForm *>(_o);
        switch (_id) {
        case 0: _t->pushButtonLoadDesign_clicked(); break;
        case 1: _t->pushButtonOpenConnection_clicked(); break;
        case 2: _t->pushButtonRun_clicked(); break;
        case 3: _t->pushButtonExamine_clicked(); break;
        case 4: _t->pushButtonSingleStepping_clicked(); break;
        case 5: _t->treeWidgetIR_itemClicked((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData mainForm::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject mainForm::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_mainForm,
      qt_meta_data_mainForm, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &mainForm::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *mainForm::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *mainForm::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_mainForm))
        return static_cast<void*>(const_cast< mainForm*>(this));
    return QDialog::qt_metacast(_clname);
}

int mainForm::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
