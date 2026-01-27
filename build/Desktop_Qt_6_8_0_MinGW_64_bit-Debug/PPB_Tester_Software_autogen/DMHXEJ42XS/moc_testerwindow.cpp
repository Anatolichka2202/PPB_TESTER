/****************************************************************************
** Meta object code from reading C++ file 'testerwindow.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../gui/testerwindow.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'testerwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSTesterWindowENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSTesterWindowENDCLASS = QtMocHelpers::stringData(
    "TesterWindow",
    "onConnectClicked",
    "",
    "onPollStatusClicked",
    "onResetClicked",
    "onApplyParametersClicked",
    "onAutoPollToggled",
    "checked",
    "onTestSequenceClicked",
    "onFUTransmitClicked",
    "onFUReceiveClicked",
    "onDisplayModeChanged",
    "showCodes",
    "onPPBSelected",
    "index",
    "onClearLogClicked",
    "onExportLogClicked",
    "onExitClicked",
    "onPultActiveClicked",
    "onPRBSM2SClicked",
    "onPRBSS2MClicked",
    "onControllerConnectionStateChanged",
    "PPBState",
    "state",
    "onControllerStatusReceived",
    "uint16_t",
    "address",
    "data",
    "onControllerErrorOccurred",
    "error",
    "onControllerLogMessage",
    "message",
    "onControllerChannelStateUpdated",
    "uint8_t",
    "ppbIndex",
    "channel",
    "UIChannelState",
    "onAutoPollToggledFromController",
    "enabled",
    "onOperationProgress",
    "current",
    "total",
    "operation",
    "onOperationCompleted",
    "success"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSTesterWindowENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      24,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  158,    2, 0x08,    1 /* Private */,
       3,    0,  159,    2, 0x08,    2 /* Private */,
       4,    0,  160,    2, 0x08,    3 /* Private */,
       5,    0,  161,    2, 0x08,    4 /* Private */,
       6,    1,  162,    2, 0x08,    5 /* Private */,
       8,    0,  165,    2, 0x08,    7 /* Private */,
       9,    0,  166,    2, 0x08,    8 /* Private */,
      10,    0,  167,    2, 0x08,    9 /* Private */,
      11,    1,  168,    2, 0x08,   10 /* Private */,
      13,    1,  171,    2, 0x08,   12 /* Private */,
      15,    0,  174,    2, 0x08,   14 /* Private */,
      16,    0,  175,    2, 0x08,   15 /* Private */,
      17,    0,  176,    2, 0x08,   16 /* Private */,
      18,    0,  177,    2, 0x08,   17 /* Private */,
      19,    0,  178,    2, 0x08,   18 /* Private */,
      20,    0,  179,    2, 0x08,   19 /* Private */,
      21,    1,  180,    2, 0x08,   20 /* Private */,
      24,    2,  183,    2, 0x08,   22 /* Private */,
      28,    1,  188,    2, 0x08,   25 /* Private */,
      30,    1,  191,    2, 0x08,   27 /* Private */,
      32,    3,  194,    2, 0x08,   29 /* Private */,
      37,    1,  201,    2, 0x08,   33 /* Private */,
      39,    3,  204,    2, 0x08,   35 /* Private */,
      43,    2,  211,    2, 0x08,   39 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   12,
    QMetaType::Void, QMetaType::Int,   14,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 22,   23,
    QMetaType::Void, 0x80000000 | 25, QMetaType::QByteArrayList,   26,   27,
    QMetaType::Void, QMetaType::QString,   29,
    QMetaType::Void, QMetaType::QString,   31,
    QMetaType::Void, 0x80000000 | 33, QMetaType::Int, 0x80000000 | 36,   34,   35,   23,
    QMetaType::Void, QMetaType::Bool,   38,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::QString,   40,   41,   42,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   44,   31,

       0        // eod
};

Q_CONSTINIT const QMetaObject TesterWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_CLASSTesterWindowENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSTesterWindowENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSTesterWindowENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<TesterWindow, std::true_type>,
        // method 'onConnectClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onPollStatusClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onResetClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onApplyParametersClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onAutoPollToggled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'onTestSequenceClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onFUTransmitClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onFUReceiveClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDisplayModeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'onPPBSelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onClearLogClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onExportLogClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onExitClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onPultActiveClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onPRBSM2SClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onPRBSS2MClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onControllerConnectionStateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<PPBState, std::false_type>,
        // method 'onControllerStatusReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector<QByteArray> &, std::false_type>,
        // method 'onControllerErrorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onControllerLogMessage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onControllerChannelStateUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint8_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const UIChannelState &, std::false_type>,
        // method 'onAutoPollToggledFromController'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'onOperationProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onOperationCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void TesterWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TesterWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onConnectClicked(); break;
        case 1: _t->onPollStatusClicked(); break;
        case 2: _t->onResetClicked(); break;
        case 3: _t->onApplyParametersClicked(); break;
        case 4: _t->onAutoPollToggled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 5: _t->onTestSequenceClicked(); break;
        case 6: _t->onFUTransmitClicked(); break;
        case 7: _t->onFUReceiveClicked(); break;
        case 8: _t->onDisplayModeChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 9: _t->onPPBSelected((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 10: _t->onClearLogClicked(); break;
        case 11: _t->onExportLogClicked(); break;
        case 12: _t->onExitClicked(); break;
        case 13: _t->onPultActiveClicked(); break;
        case 14: _t->onPRBSM2SClicked(); break;
        case 15: _t->onPRBSS2MClicked(); break;
        case 16: _t->onControllerConnectionStateChanged((*reinterpret_cast< std::add_pointer_t<PPBState>>(_a[1]))); break;
        case 17: _t->onControllerStatusReceived((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QList<QByteArray>>>(_a[2]))); break;
        case 18: _t->onControllerErrorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 19: _t->onControllerLogMessage((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 20: _t->onControllerChannelStateUpdated((*reinterpret_cast< std::add_pointer_t<uint8_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<UIChannelState>>(_a[3]))); break;
        case 21: _t->onAutoPollToggledFromController((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 22: _t->onOperationProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 23: _t->onOperationCompleted((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObject *TesterWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TesterWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSTesterWindowENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int TesterWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 24)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 24;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 24)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 24;
    }
    return _id;
}
QT_WARNING_POP
