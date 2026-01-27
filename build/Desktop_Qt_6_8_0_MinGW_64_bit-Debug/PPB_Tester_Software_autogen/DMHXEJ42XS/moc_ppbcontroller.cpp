/****************************************************************************
** Meta object code from reading C++ file 'ppbcontroller.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../gui/ppbcontroller.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ppbcontroller.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSPPBControllerENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSPPBControllerENDCLASS = QtMocHelpers::stringData(
    "PPBController",
    "executeCommandRequested",
    "",
    "TechCommand",
    "cmd",
    "uint16_t",
    "address",
    "connectionStateChanged",
    "PPBState",
    "state",
    "busyChanged",
    "busy",
    "statusReceived",
    "data",
    "errorOccurred",
    "error",
    "logMessage",
    "message",
    "channelStateUpdated",
    "uint8_t",
    "ppbIndex",
    "channel",
    "UIChannelState",
    "autoPollToggled",
    "enabled",
    "connectToPPBSignal",
    "ip",
    "port",
    "disconnectSignal",
    "sendFUReceiveSignal",
    "period",
    "const uint8_t[3]",
    "fuData",
    "sendFUTransmitSignal",
    "addres",
    "operationProgress",
    "current",
    "total",
    "operation",
    "operationCompleted",
    "success",
    "onStatusReceived",
    "onConnectionStateChanged",
    "onCommandProgress",
    "command",
    "onCommandCompleted",
    "onErrorOccurred",
    "onAutoPollTimeout",
    "onBusyChanged",
    "connectToPPB",
    "disconnect",
    "requestStatus",
    "resetPPB",
    "setGeneratorParameters",
    "uint32_t",
    "duration",
    "duty",
    "delay",
    "setFUReceive",
    "setFUTransmit",
    "startPRBS_M2S",
    "startPRBS_S2M",
    "runFullTest",
    "startAutoPoll",
    "intervalMs",
    "stopAutoPoll",
    "connectionState",
    "isBusy",
    "isAutoPollEnabled",
    "getChannelState",
    "requestVersion",
    "requestVolume",
    "requestChecksum",
    "sendProgram",
    "sendClean",
    "requestDroppedPackets",
    "requestBER_T",
    "requestBER_F"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSPPBControllerENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      47,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      14,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,  296,    2, 0x06,    1 /* Public */,
       7,    1,  301,    2, 0x06,    4 /* Public */,
      10,    1,  304,    2, 0x06,    6 /* Public */,
      12,    2,  307,    2, 0x06,    8 /* Public */,
      14,    1,  312,    2, 0x06,   11 /* Public */,
      16,    1,  315,    2, 0x06,   13 /* Public */,
      18,    3,  318,    2, 0x06,   15 /* Public */,
      23,    1,  325,    2, 0x06,   19 /* Public */,
      25,    3,  328,    2, 0x06,   21 /* Public */,
      28,    0,  335,    2, 0x06,   25 /* Public */,
      29,    3,  336,    2, 0x06,   26 /* Public */,
      33,    1,  343,    2, 0x06,   30 /* Public */,
      35,    3,  346,    2, 0x06,   32 /* Public */,
      39,    2,  353,    2, 0x06,   36 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      41,    2,  358,    2, 0x08,   39 /* Private */,
      42,    1,  363,    2, 0x08,   42 /* Private */,
      43,    3,  366,    2, 0x08,   44 /* Private */,
      45,    3,  373,    2, 0x08,   48 /* Private */,
      46,    1,  380,    2, 0x08,   52 /* Private */,
      47,    0,  383,    2, 0x08,   54 /* Private */,
      48,    1,  384,    2, 0x08,   55 /* Private */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
      49,    3,  387,    2, 0x02,   57 /* Public */,
      50,    0,  394,    2, 0x02,   61 /* Public */,
      51,    1,  395,    2, 0x02,   62 /* Public */,
      52,    1,  398,    2, 0x02,   64 /* Public */,
      53,    4,  401,    2, 0x02,   66 /* Public */,
      58,    2,  410,    2, 0x02,   71 /* Public */,
      58,    1,  415,    2, 0x22,   74 /* Public | MethodCloned */,
      59,    1,  418,    2, 0x02,   76 /* Public */,
      60,    1,  421,    2, 0x02,   78 /* Public */,
      61,    1,  424,    2, 0x02,   80 /* Public */,
      62,    1,  427,    2, 0x02,   82 /* Public */,
      63,    1,  430,    2, 0x02,   84 /* Public */,
      63,    0,  433,    2, 0x22,   86 /* Public | MethodCloned */,
      65,    0,  434,    2, 0x02,   87 /* Public */,
      66,    0,  435,    2, 0x102,   88 /* Public | MethodIsConst  */,
      67,    0,  436,    2, 0x102,   89 /* Public | MethodIsConst  */,
      68,    0,  437,    2, 0x102,   90 /* Public | MethodIsConst  */,
      69,    2,  438,    2, 0x102,   91 /* Public | MethodIsConst  */,
      70,    1,  443,    2, 0x02,   94 /* Public */,
      71,    1,  446,    2, 0x02,   96 /* Public */,
      72,    1,  449,    2, 0x02,   98 /* Public */,
      73,    1,  452,    2, 0x02,  100 /* Public */,
      74,    1,  455,    2, 0x02,  102 /* Public */,
      75,    1,  458,    2, 0x02,  104 /* Public */,
      76,    1,  461,    2, 0x02,  106 /* Public */,
      77,    1,  464,    2, 0x02,  108 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5,    4,    6,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, QMetaType::Bool,   11,
    QMetaType::Void, 0x80000000 | 5, QMetaType::QByteArrayList,    6,   13,
    QMetaType::Void, QMetaType::QString,   15,
    QMetaType::Void, QMetaType::QString,   17,
    QMetaType::Void, 0x80000000 | 19, QMetaType::Int, 0x80000000 | 22,   20,   21,    9,
    QMetaType::Void, QMetaType::Bool,   24,
    QMetaType::Void, 0x80000000 | 5, QMetaType::QString, QMetaType::UShort,    6,   26,   27,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 19, 0x80000000 | 31,    6,   30,   32,
    QMetaType::Void, 0x80000000 | 5,   34,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::QString,   36,   37,   38,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   40,   17,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 5, QMetaType::QByteArrayList,    6,   13,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, 0x80000000 | 3,   36,   37,   44,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString, 0x80000000 | 3,   40,   17,   44,
    QMetaType::Void, QMetaType::QString,   15,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   11,

 // methods: parameters
    QMetaType::Void, 0x80000000 | 5, QMetaType::QString, QMetaType::UShort,    6,   26,   27,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 54, 0x80000000 | 19, 0x80000000 | 54,    6,   55,   56,   57,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 19,    6,   30,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, QMetaType::Int,   64,
    QMetaType::Void,
    QMetaType::Void,
    0x80000000 | 8,
    QMetaType::Bool,
    QMetaType::Bool,
    0x80000000 | 22, 0x80000000 | 19, QMetaType::Int,   20,   21,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 5,    6,

       0        // eod
};

Q_CONSTINIT const QMetaObject PPBController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSPPBControllerENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSPPBControllerENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSPPBControllerENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<PPBController, std::true_type>,
        // method 'executeCommandRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<TechCommand, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        // method 'connectionStateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<PPBState, std::false_type>,
        // method 'busyChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'statusReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector<QByteArray> &, std::false_type>,
        // method 'errorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'logMessage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'channelStateUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint8_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const UIChannelState &, std::false_type>,
        // method 'autoPollToggled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'connectToPPBSignal'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<quint16, std::false_type>,
        // method 'disconnectSignal'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'sendFUReceiveSignal'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint8_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const uint8_t, std::false_type>,
        // method 'sendFUTransmitSignal'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        // method 'operationProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'operationCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onStatusReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector<QByteArray> &, std::false_type>,
        // method 'onConnectionStateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<PPBState, std::false_type>,
        // method 'onCommandProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<TechCommand, std::false_type>,
        // method 'onCommandCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<TechCommand, std::false_type>,
        // method 'onErrorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onAutoPollTimeout'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onBusyChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'connectToPPB'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<quint16, std::false_type>,
        // method 'disconnect'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'requestStatus'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        // method 'resetPPB'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        // method 'setGeneratorParameters'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint32_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint8_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint32_t, std::false_type>,
        // method 'setFUReceive'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint8_t, std::false_type>,
        // method 'setFUReceive'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        // method 'setFUTransmit'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        // method 'startPRBS_M2S'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        // method 'startPRBS_S2M'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        // method 'runFullTest'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        // method 'startAutoPoll'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'startAutoPoll'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'stopAutoPoll'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'connectionState'
        QtPrivate::TypeAndForceComplete<PPBState, std::false_type>,
        // method 'isBusy'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'isAutoPollEnabled'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'getChannelState'
        QtPrivate::TypeAndForceComplete<UIChannelState, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint8_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'requestVersion'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        // method 'requestVolume'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        // method 'requestChecksum'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        // method 'sendProgram'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        // method 'sendClean'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        // method 'requestDroppedPackets'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        // method 'requestBER_T'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        // method 'requestBER_F'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>
    >,
    nullptr
} };

void PPBController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PPBController *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->executeCommandRequested((*reinterpret_cast< std::add_pointer_t<TechCommand>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[2]))); break;
        case 1: _t->connectionStateChanged((*reinterpret_cast< std::add_pointer_t<PPBState>>(_a[1]))); break;
        case 2: _t->busyChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 3: _t->statusReceived((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QList<QByteArray>>>(_a[2]))); break;
        case 4: _t->errorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->logMessage((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->channelStateUpdated((*reinterpret_cast< std::add_pointer_t<uint8_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<UIChannelState>>(_a[3]))); break;
        case 7: _t->autoPollToggled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 8: _t->connectToPPBSignal((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<quint16>>(_a[3]))); break;
        case 9: _t->disconnectSignal(); break;
        case 10: _t->sendFUReceiveSignal((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<uint8_t>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<const uint8_t[3]>>(_a[3]))); break;
        case 11: _t->sendFUTransmitSignal((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1]))); break;
        case 12: _t->operationProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 13: _t->operationCompleted((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 14: _t->onStatusReceived((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QList<QByteArray>>>(_a[2]))); break;
        case 15: _t->onConnectionStateChanged((*reinterpret_cast< std::add_pointer_t<PPBState>>(_a[1]))); break;
        case 16: _t->onCommandProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<TechCommand>>(_a[3]))); break;
        case 17: _t->onCommandCompleted((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<TechCommand>>(_a[3]))); break;
        case 18: _t->onErrorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 19: _t->onAutoPollTimeout(); break;
        case 20: _t->onBusyChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 21: _t->connectToPPB((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<quint16>>(_a[3]))); break;
        case 22: _t->disconnect(); break;
        case 23: _t->requestStatus((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1]))); break;
        case 24: _t->resetPPB((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1]))); break;
        case 25: _t->setGeneratorParameters((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<uint8_t>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[4]))); break;
        case 26: _t->setFUReceive((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<uint8_t>>(_a[2]))); break;
        case 27: _t->setFUReceive((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1]))); break;
        case 28: _t->setFUTransmit((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1]))); break;
        case 29: _t->startPRBS_M2S((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1]))); break;
        case 30: _t->startPRBS_S2M((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1]))); break;
        case 31: _t->runFullTest((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1]))); break;
        case 32: _t->startAutoPoll((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 33: _t->startAutoPoll(); break;
        case 34: _t->stopAutoPoll(); break;
        case 35: { PPBState _r = _t->connectionState();
            if (_a[0]) *reinterpret_cast< PPBState*>(_a[0]) = std::move(_r); }  break;
        case 36: { bool _r = _t->isBusy();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 37: { bool _r = _t->isAutoPollEnabled();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 38: { UIChannelState _r = _t->getChannelState((*reinterpret_cast< std::add_pointer_t<uint8_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])));
            if (_a[0]) *reinterpret_cast< UIChannelState*>(_a[0]) = std::move(_r); }  break;
        case 39: _t->requestVersion((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1]))); break;
        case 40: _t->requestVolume((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1]))); break;
        case 41: _t->requestChecksum((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1]))); break;
        case 42: _t->sendProgram((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1]))); break;
        case 43: _t->sendClean((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1]))); break;
        case 44: _t->requestDroppedPackets((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1]))); break;
        case 45: _t->requestBER_T((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1]))); break;
        case 46: _t->requestBER_F((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PPBController::*)(TechCommand , uint16_t );
            if (_t _q_method = &PPBController::executeCommandRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (PPBController::*)(PPBState );
            if (_t _q_method = &PPBController::connectionStateChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (PPBController::*)(bool );
            if (_t _q_method = &PPBController::busyChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (PPBController::*)(uint16_t , const QVector<QByteArray> & );
            if (_t _q_method = &PPBController::statusReceived; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (PPBController::*)(const QString & );
            if (_t _q_method = &PPBController::errorOccurred; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (PPBController::*)(const QString & );
            if (_t _q_method = &PPBController::logMessage; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (PPBController::*)(uint8_t , int , const UIChannelState & );
            if (_t _q_method = &PPBController::channelStateUpdated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (PPBController::*)(bool );
            if (_t _q_method = &PPBController::autoPollToggled; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (PPBController::*)(uint16_t , const QString & , quint16 );
            if (_t _q_method = &PPBController::connectToPPBSignal; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (PPBController::*)();
            if (_t _q_method = &PPBController::disconnectSignal; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (PPBController::*)(uint16_t , uint8_t , const uint8_t [3]);
            if (_t _q_method = &PPBController::sendFUReceiveSignal; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 10;
                return;
            }
        }
        {
            using _t = void (PPBController::*)(uint16_t );
            if (_t _q_method = &PPBController::sendFUTransmitSignal; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 11;
                return;
            }
        }
        {
            using _t = void (PPBController::*)(int , int , const QString & );
            if (_t _q_method = &PPBController::operationProgress; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 12;
                return;
            }
        }
        {
            using _t = void (PPBController::*)(bool , const QString & );
            if (_t _q_method = &PPBController::operationCompleted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 13;
                return;
            }
        }
    }
}

const QMetaObject *PPBController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PPBController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSPPBControllerENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PPBController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 47)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 47;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 47)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 47;
    }
    return _id;
}

// SIGNAL 0
void PPBController::executeCommandRequested(TechCommand _t1, uint16_t _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PPBController::connectionStateChanged(PPBState _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void PPBController::busyChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void PPBController::statusReceived(uint16_t _t1, const QVector<QByteArray> & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void PPBController::errorOccurred(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void PPBController::logMessage(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void PPBController::channelStateUpdated(uint8_t _t1, int _t2, const UIChannelState & _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void PPBController::autoPollToggled(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void PPBController::connectToPPBSignal(uint16_t _t1, const QString & _t2, quint16 _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void PPBController::disconnectSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void PPBController::sendFUReceiveSignal(uint16_t _t1, uint8_t _t2, const uint8_t _t3[3])
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void PPBController::sendFUTransmitSignal(uint16_t _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 11, _a);
}

// SIGNAL 12
void PPBController::operationProgress(int _t1, int _t2, const QString & _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}

// SIGNAL 13
void PPBController::operationCompleted(bool _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 13, _a);
}
QT_WARNING_POP
