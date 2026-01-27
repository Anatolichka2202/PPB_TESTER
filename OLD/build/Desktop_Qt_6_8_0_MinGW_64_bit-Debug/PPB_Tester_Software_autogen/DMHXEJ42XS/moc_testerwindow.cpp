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
    "onLoadScenarioClicked",
    "onRunScenarioClicked",
    "onStopScenarioClicked",
    "onDisplayModeChanged",
    "showCodes",
    "onPPBSelected",
    "index",
    "onClearLogClicked",
    "onExportLogClicked",
    "onExitClicked",
    "onConnectionStateChanged",
    "PPBState",
    "state",
    "onBusyChanged",
    "busy",
    "onConnected",
    "onDisconnected",
    "onStatusReceived",
    "uint16_t",
    "address",
    "data",
    "onTestProgress",
    "current",
    "total",
    "OperationType",
    "operation",
    "onTestCompleted",
    "success",
    "report",
    "onTestPacketReceived",
    "DataPacket",
    "packet",
    "onErrorOccurred",
    "error",
    "onTimeoutOccurred",
    "onLogMessage",
    "message",
    "onAutoPollTimeout",
    "onUIUpdateTimeout"
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
      29,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  188,    2, 0x08,    1 /* Private */,
       3,    0,  189,    2, 0x08,    2 /* Private */,
       4,    0,  190,    2, 0x08,    3 /* Private */,
       5,    0,  191,    2, 0x08,    4 /* Private */,
       6,    1,  192,    2, 0x08,    5 /* Private */,
       8,    0,  195,    2, 0x08,    7 /* Private */,
       9,    0,  196,    2, 0x08,    8 /* Private */,
      10,    0,  197,    2, 0x08,    9 /* Private */,
      11,    0,  198,    2, 0x08,   10 /* Private */,
      12,    0,  199,    2, 0x08,   11 /* Private */,
      13,    0,  200,    2, 0x08,   12 /* Private */,
      14,    1,  201,    2, 0x08,   13 /* Private */,
      16,    1,  204,    2, 0x08,   15 /* Private */,
      18,    0,  207,    2, 0x08,   17 /* Private */,
      19,    0,  208,    2, 0x08,   18 /* Private */,
      20,    0,  209,    2, 0x08,   19 /* Private */,
      21,    1,  210,    2, 0x08,   20 /* Private */,
      24,    1,  213,    2, 0x08,   22 /* Private */,
      26,    0,  216,    2, 0x08,   24 /* Private */,
      27,    0,  217,    2, 0x08,   25 /* Private */,
      28,    2,  218,    2, 0x08,   26 /* Private */,
      32,    3,  223,    2, 0x08,   29 /* Private */,
      37,    3,  230,    2, 0x08,   33 /* Private */,
      40,    1,  237,    2, 0x08,   37 /* Private */,
      43,    1,  240,    2, 0x08,   39 /* Private */,
      45,    1,  243,    2, 0x08,   41 /* Private */,
      46,    1,  246,    2, 0x08,   43 /* Private */,
      48,    0,  249,    2, 0x08,   45 /* Private */,
      49,    0,  250,    2, 0x08,   46 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   15,
    QMetaType::Void, QMetaType::Int,   17,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 22,   23,
    QMetaType::Void, QMetaType::Bool,   25,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 29, QMetaType::QByteArrayList,   30,   31,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, 0x80000000 | 35,   33,   34,   36,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString, 0x80000000 | 35,   38,   39,   36,
    QMetaType::Void, 0x80000000 | 41,   42,
    QMetaType::Void, QMetaType::QString,   44,
    QMetaType::Void, QMetaType::QString,   36,
    QMetaType::Void, QMetaType::QString,   47,
    QMetaType::Void,
    QMetaType::Void,

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
        // method 'onLoadScenarioClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onRunScenarioClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onStopScenarioClicked'
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
        // method 'onConnectionStateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<PPBState, std::false_type>,
        // method 'onBusyChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'onConnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDisconnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onStatusReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector<QByteArray> &, std::false_type>,
        // method 'onTestProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<OperationType, std::false_type>,
        // method 'onTestCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<OperationType, std::false_type>,
        // method 'onTestPacketReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const DataPacket &, std::false_type>,
        // method 'onErrorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onTimeoutOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onLogMessage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onAutoPollTimeout'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onUIUpdateTimeout'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
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
        case 8: _t->onLoadScenarioClicked(); break;
        case 9: _t->onRunScenarioClicked(); break;
        case 10: _t->onStopScenarioClicked(); break;
        case 11: _t->onDisplayModeChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 12: _t->onPPBSelected((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 13: _t->onClearLogClicked(); break;
        case 14: _t->onExportLogClicked(); break;
        case 15: _t->onExitClicked(); break;
        case 16: _t->onConnectionStateChanged((*reinterpret_cast< std::add_pointer_t<PPBState>>(_a[1]))); break;
        case 17: _t->onBusyChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 18: _t->onConnected(); break;
        case 19: _t->onDisconnected(); break;
        case 20: _t->onStatusReceived((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QList<QByteArray>>>(_a[2]))); break;
        case 21: _t->onTestProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<OperationType>>(_a[3]))); break;
        case 22: _t->onTestCompleted((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<OperationType>>(_a[3]))); break;
        case 23: _t->onTestPacketReceived((*reinterpret_cast< std::add_pointer_t<DataPacket>>(_a[1]))); break;
        case 24: _t->onErrorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 25: _t->onTimeoutOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 26: _t->onLogMessage((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 27: _t->onAutoPollTimeout(); break;
        case 28: _t->onUIUpdateTimeout(); break;
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
        if (_id < 29)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 29;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 29)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 29;
    }
    return _id;
}
QT_WARNING_POP
