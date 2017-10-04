/****************************************************************************
** Meta object code from reading C++ file 'formMain.h'
**
** Created: Mon Jul 14 11:11:33 2014
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "formMain.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'formMain.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_formMain[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      23,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x0a,
      41,    9,    9,    9, 0x0a,
      76,    9,    9,    9, 0x0a,
     111,    9,    9,    9, 0x0a,
     138,    9,    9,    9, 0x0a,
     169,    9,    9,    9, 0x0a,
     211,  204,    9,    9, 0x0a,
     258,    9,    9,    9, 0x0a,
     283,    9,    9,    9, 0x0a,
     320,    9,    9,    9, 0x0a,
     349,    9,    9,    9, 0x0a,
     378,  376,    9,    9, 0x0a,
     412,    9,    9,    9, 0x0a,
     454,    9,    9,    9, 0x0a,
     498,    9,    9,    9, 0x0a,
     527,    9,    9,    9, 0x0a,
     555,    9,    9,    9, 0x0a,
     581,    9,    9,    9, 0x0a,
     613,    9,    9,    9, 0x0a,
     643,    9,    9,    9, 0x0a,
     673,    9,    9,    9, 0x0a,
     714,    9,    9,    9, 0x0a,
     780,  763,    9,    9, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_formMain[] = {
    "formMain\0\0pushButtonLoadDesign_clicked()\0"
    "pushButtonOpenConnection_clicked()\0"
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

const QMetaObject formMain::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_formMain,
      qt_meta_data_formMain, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &formMain::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *formMain::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *formMain::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_formMain))
        return static_cast<void*>(const_cast< formMain*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int formMain::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: pushButtonLoadDesign_clicked(); break;
        case 1: pushButtonOpenConnection_clicked(); break;
        case 2: pushButtonSingleStepping_clicked(); break;
        case 3: singleSteppingForGDBSync(); break;
        case 4: singleSteppingForPureGDBMode(); break;
        case 5: singleSteppingForGDBBugDetection(); break;
        case 6: treeWidgetIR_itemClicked((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 7: pushButtonExit_clicked(); break;
        case 8: actionView_IR_Instructions_changed(); break;
        case 9: actionView_HW_Info_changed(); break;
        case 10: actionView_Watch_changed(); break;
        case 11: radioButtonStepInto_Toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 12: textEditHighLevel_CursorPositionChanged(); break;
        case 13: textEditLineNumbers_CursorPositionChanged(); break;
        case 14: pushButtonContinue_clicked(); break;
        case 15: actionReBuildCode_clicked(); break;
        case 16: actionRunOnChip_clicked(); break;
        case 17: actionRunReferenceSim_clicked(); break;
        case 18: loadReferenceSimulationData(); break;
        case 19: actionSelectSignals_clicked(); break;
        case 20: actionAutomaticSignalSelection_clicked(); break;
        case 21: plainTableEditLog_itemClicked((*reinterpret_cast< QTableWidgetItem*(*)>(_a[1]))); break;
        case 22: HighlightBugInCCode((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 23;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
