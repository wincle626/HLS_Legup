/****************************************************************************
** Meta object code from reading C++ file 'newForm.h'
**
** Created: Mon Jul 7 10:48:36 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "newForm.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'newForm.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_newForm[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      25,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
       9,    8,    8,    8, 0x0a,
      40,    8,    8,    8, 0x0a,
      75,    8,    8,    8, 0x0a,
      99,    8,    8,    8, 0x0a,
     127,    8,    8,    8, 0x0a,
     162,    8,    8,    8, 0x0a,
     189,    8,    8,    8, 0x0a,
     220,    8,    8,    8, 0x0a,
     262,  255,    8,    8, 0x0a,
     309,    8,    8,    8, 0x0a,
     334,    8,    8,    8, 0x0a,
     371,    8,    8,    8, 0x0a,
     400,    8,    8,    8, 0x0a,
     429,  427,    8,    8, 0x0a,
     463,    8,    8,    8, 0x0a,
     505,    8,    8,    8, 0x0a,
     549,    8,    8,    8, 0x0a,
     578,    8,    8,    8, 0x0a,
     606,    8,    8,    8, 0x0a,
     632,    8,    8,    8, 0x0a,
     664,    8,    8,    8, 0x0a,
     694,    8,    8,    8, 0x0a,
     724,    8,    8,    8, 0x0a,
     765,    8,    8,    8, 0x0a,
     831,  814,    8,    8, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_newForm[] = {
    "newForm\0\0pushButtonLoadDesign_clicked()\0"
    "pushButtonOpenConnection_clicked()\0"
    "pushButtonRun_clicked()\0"
    "pushButtonExamine_clicked()\0"
    "pushButtonSingleStepping_clicked()\0"
    "singleSteppingForGDBSync()\0"
    "singleSteppingForPureGDBMode()\0"
    "singleSteppingForGDBBugDetection()\0"
    "item,c\0treeWidgetIR_itemClicked(QTreeWidgetItem*,int)\0"
    "pushButtonExit_clicked()\0"
    "actionView_IR_Instructions_changed()\0"
    "actionView_HW_Info_changed()\0"
    "actionView_Watch_changed()\0b\0"
    "radioButtonStepInto_Toggled(bool)\0"
    "textEditHighLevel_CursorPositionChanged()\0"
    "textEditLineNumbers_CursorPositionChanged()\0"
    "pushButtonContinue_clicked()\0"
    "actionReBuildCode_clicked()\0"
    "actionRunOnChip_clicked()\0"
    "actionRunReferenceSim_clicked()\0"
    "loadReferenceSimulationData()\0"
    "actionSelectSignals_clicked()\0"
    "actionAutomaticSignalSelection_clicked()\0"
    "plainTableEditLog_itemClicked(QTableWidgetItem*)\0"
    "highlightingLine\0HighlightBugInCCode(int)\0"
};

void newForm::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        newForm *_t = static_cast<newForm *>(_o);
        switch (_id) {
        case 0: _t->pushButtonLoadDesign_clicked(); break;
        case 1: _t->pushButtonOpenConnection_clicked(); break;
        case 2: _t->pushButtonRun_clicked(); break;
        case 3: _t->pushButtonExamine_clicked(); break;
        case 4: _t->pushButtonSingleStepping_clicked(); break;
        case 5: _t->singleSteppingForGDBSync(); break;
        case 6: _t->singleSteppingForPureGDBMode(); break;
        case 7: _t->singleSteppingForGDBBugDetection(); break;
        case 8: _t->treeWidgetIR_itemClicked((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 9: _t->pushButtonExit_clicked(); break;
        case 10: _t->actionView_IR_Instructions_changed(); break;
        case 11: _t->actionView_HW_Info_changed(); break;
        case 12: _t->actionView_Watch_changed(); break;
        case 13: _t->radioButtonStepInto_Toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 14: _t->textEditHighLevel_CursorPositionChanged(); break;
        case 15: _t->textEditLineNumbers_CursorPositionChanged(); break;
        case 16: _t->pushButtonContinue_clicked(); break;
        case 17: _t->actionReBuildCode_clicked(); break;
        case 18: _t->actionRunOnChip_clicked(); break;
        case 19: _t->actionRunReferenceSim_clicked(); break;
        case 20: _t->loadReferenceSimulationData(); break;
        case 21: _t->actionSelectSignals_clicked(); break;
        case 22: _t->actionAutomaticSignalSelection_clicked(); break;
        case 23: _t->plainTableEditLog_itemClicked((*reinterpret_cast< QTableWidgetItem*(*)>(_a[1]))); break;
        case 24: _t->HighlightBugInCCode((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData newForm::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject newForm::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_newForm,
      qt_meta_data_newForm, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &newForm::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *newForm::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *newForm::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_newForm))
        return static_cast<void*>(const_cast< newForm*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int newForm::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 25)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 25;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
