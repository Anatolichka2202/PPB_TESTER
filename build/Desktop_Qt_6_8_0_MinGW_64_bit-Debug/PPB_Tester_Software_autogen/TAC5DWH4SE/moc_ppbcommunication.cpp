/****************************************************************************
** Meta object code from reading C++ file 'ppbcommunication.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../core/ppbcommunication.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ppbcommunication.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSPPBCommunicationENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSPPBCommunicationENDCLASS = QtMocHelpers::stringData(
    "PPBCommunication",
    "stateChanged",
    "",
    "PPBState",
    "state",
    "connected",
    "disconnected",
    "busyChange",
    "busy",
    "statusReceived",
    "uint16_t",
    "address",
    "data",
    "commandProgress",
    "current",
    "total",
    "TechCommand",
    "command",
    "commandCompleted",
    "success",
    "report",
    "errorOccurred",
    "error",
    "logMessage",
    "message",
    "initialized",
    "sendDataPackets",
    "QList<DataPacket>",
    "packets",
    "getGeneratedPackets",
    "stop",
    "enqueueCommand",
    "cmd",
    "initialize",
    "UDPClient*",
    "udpClient",
    "onDataReceived",
    "QHostAddress",
    "sender",
    "port",
    "onNetworkError",
    "onOperationTimeout",
    "sendNextPacket",
    "setUDPClient",
    "processNextTask",
    "initializeInThread",
    "connectToPPB",
    "ip",
    "disconnect",
    "executeCommand",
    "sendFUTransmit",
    "sendFUReceive",
    "uint8_t",
    "period",
    "const uint8_t[3]",
    "fuData"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSPPBCommunicationENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      28,   14, // methods
       1,  274, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      10,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,  182,    2, 0x06,    2 /* Public */,
       5,    0,  185,    2, 0x06,    4 /* Public */,
       6,    0,  186,    2, 0x06,    5 /* Public */,
       7,    1,  187,    2, 0x06,    6 /* Public */,
       9,    2,  190,    2, 0x06,    8 /* Public */,
      13,    3,  195,    2, 0x06,   11 /* Public */,
      18,    3,  202,    2, 0x06,   15 /* Public */,
      21,    1,  209,    2, 0x06,   19 /* Public */,
      23,    1,  212,    2, 0x06,   21 /* Public */,
      25,    0,  215,    2, 0x06,   23 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      26,    1,  216,    2, 0x0a,   24 /* Public */,
      29,    0,  219,    2, 0x10a,   26 /* Public | MethodIsConst  */,
      30,    0,  220,    2, 0x0a,   27 /* Public */,
      31,    2,  221,    2, 0x0a,   28 /* Public */,
      33,    1,  226,    2, 0x0a,   31 /* Public */,
      36,    3,  229,    2, 0x08,   33 /* Private */,
      40,    1,  236,    2, 0x08,   37 /* Private */,
      41,    0,  239,    2, 0x08,   39 /* Private */,
      42,    0,  240,    2, 0x08,   40 /* Private */,
      43,    1,  241,    2, 0x08,   41 /* Private */,
      44,    0,  244,    2, 0x08,   43 /* Private */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
      45,    0,  245,    2, 0x02,   44 /* Public */,
      46,    3,  246,    2, 0x02,   45 /* Public */,
      48,    0,  253,    2, 0x02,   49 /* Public */,
      49,    2,  254,    2, 0x02,   50 /* Public */,
      50,    1,  259,    2, 0x02,   53 /* Public */,
      51,    3,  262,    2, 0x02,   55 /* Public */,
      51,    2,  269,    2, 0x22,   59 /* Public | MethodCloned */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    8,
    QMetaType::Void, 0x80000000 | 10, QMetaType::QByteArrayList,   11,   12,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, 0x80000000 | 16,   14,   15,   17,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString, 0x80000000 | 16,   19,   20,   17,
    QMetaType::Void, QMetaType::QString,   22,
    QMetaType::Void, QMetaType::QString,   24,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 27,   28,
    0x80000000 | 27,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 16, 0x80000000 | 10,   32,   11,
    QMetaType::Void, 0x80000000 | 34,   35,
    QMetaType::Void, QMetaType::QByteArray, 0x80000000 | 37, QMetaType::UShort,   12,   38,   39,
    QMetaType::Void, QMetaType::QString,   22,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 34,   35,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void,
    QMetaType::Bool, 0x80000000 | 10, QMetaType::QString, QMetaType::UShort,   11,   47,   39,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 16, 0x80000000 | 10,   32,   11,
    QMetaType::Void, 0x80000000 | 10,   11,
    QMetaType::Void, 0x80000000 | 10, 0x80000000 | 52, 0x80000000 | 54,   11,   53,   55,
    QMetaType::Void, 0x80000000 | 10, 0x80000000 | 52,   11,   53,

 // properties: name, type, flags, notifyId, revision
       4, 0x80000000 | 3, 0x00015009, uint(0), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject PPBCommunication::staticMetaObject = { {
    QMetaObject::SuperData::link<CommandInterface::staticMetaObject>(),
    qt_meta_stringdata_CLASSPPBCommunicationENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSPPBCommunicationENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSPPBCommunicationENDCLASS_t,
        // property 'state'
        QtPrivate::TypeAndForceComplete<PPBState, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<PPBCommunication, std::true_type>,
        // method 'stateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<PPBState, std::false_type>,
        // method 'connected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'disconnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'busyChange'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'statusReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector<QByteArray> &, std::false_type>,
        // method 'commandProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<TechCommand, std::false_type>,
        // method 'commandCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<TechCommand, std::false_type>,
        // method 'errorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'logMessage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'initialized'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'sendDataPackets'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector<DataPacket> &, std::false_type>,
        // method 'getGeneratedPackets'
        QtPrivate::TypeAndForceComplete<QVector<DataPacket>, std::false_type>,
        // method 'stop'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'enqueueCommand'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<TechCommand, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        // method 'initialize'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<UDPClient *, std::false_type>,
        // method 'onDataReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QHostAddress &, std::false_type>,
        QtPrivate::TypeAndForceComplete<quint16, std::false_type>,
        // method 'onNetworkError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onOperationTimeout'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'sendNextPacket'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'setUDPClient'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<UDPClient *, std::false_type>,
        // method 'processNextTask'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'initializeInThread'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'connectToPPB'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<quint16, std::false_type>,
        // method 'disconnect'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'executeCommand'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<TechCommand, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        // method 'sendFUTransmit'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        // method 'sendFUReceive'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint8_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const uint8_t, std::false_type>,
        // method 'sendFUReceive'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint16_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint8_t, std::false_type>
    >,
    nullptr
} };

void PPBCommunication::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PPBCommunication *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->stateChanged((*reinterpret_cast< std::add_pointer_t<PPBState>>(_a[1]))); break;
        case 1: _t->connected(); break;
        case 2: _t->disconnected(); break;
        case 3: _t->busyChange((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 4: _t->statusReceived((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QList<QByteArray>>>(_a[2]))); break;
        case 5: _t->commandProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<TechCommand>>(_a[3]))); break;
        case 6: _t->commandCompleted((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<TechCommand>>(_a[3]))); break;
        case 7: _t->errorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->logMessage((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 9: _t->initialized(); break;
        case 10: _t->sendDataPackets((*reinterpret_cast< std::add_pointer_t<QList<DataPacket>>>(_a[1]))); break;
        case 11: { QList<DataPacket> _r = _t->getGeneratedPackets();
            if (_a[0]) *reinterpret_cast< QList<DataPacket>*>(_a[0]) = std::move(_r); }  break;
        case 12: _t->stop(); break;
        case 13: _t->enqueueCommand((*reinterpret_cast< std::add_pointer_t<TechCommand>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[2]))); break;
        case 14: _t->initialize((*reinterpret_cast< std::add_pointer_t<UDPClient*>>(_a[1]))); break;
        case 15: _t->onDataReceived((*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QHostAddress>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<quint16>>(_a[3]))); break;
        case 16: _t->onNetworkError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 17: _t->onOperationTimeout(); break;
        case 18: _t->sendNextPacket(); break;
        case 19: _t->setUDPClient((*reinterpret_cast< std::add_pointer_t<UDPClient*>>(_a[1]))); break;
        case 20: _t->processNextTask(); break;
        case 21: _t->initializeInThread(); break;
        case 22: { bool _r = _t->connectToPPB((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<quint16>>(_a[3])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 23: _t->disconnect(); break;
        case 24: _t->executeCommand((*reinterpret_cast< std::add_pointer_t<TechCommand>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[2]))); break;
        case 25: _t->sendFUTransmit((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1]))); break;
        case 26: _t->sendFUReceive((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<uint8_t>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<const uint8_t[3]>>(_a[3]))); break;
        case 27: _t->sendFUReceive((*reinterpret_cast< std::add_pointer_t<uint16_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<uint8_t>>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 14:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< UDPClient* >(); break;
            }
            break;
        case 19:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< UDPClient* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PPBCommunication::*)(PPBState );
            if (_t _q_method = &PPBCommunication::stateChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (PPBCommunication::*)();
            if (_t _q_method = &PPBCommunication::connected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (PPBCommunication::*)();
            if (_t _q_method = &PPBCommunication::disconnected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (PPBCommunication::*)(bool );
            if (_t _q_method = &PPBCommunication::busyChange; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (PPBCommunication::*)(uint16_t , const QVector<QByteArray> & );
            if (_t _q_method = &PPBCommunication::statusReceived; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (PPBCommunication::*)(int , int , TechCommand );
            if (_t _q_method = &PPBCommunication::commandProgress; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (PPBCommunication::*)(bool , const QString & , TechCommand );
            if (_t _q_method = &PPBCommunication::commandCompleted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (PPBCommunication::*)(const QString & );
            if (_t _q_method = &PPBCommunication::errorOccurred; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (PPBCommunication::*)(const QString & );
            if (_t _q_method = &PPBCommunication::logMessage; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (PPBCommunication::*)();
            if (_t _q_method = &PPBCommunication::initialized; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 9;
                return;
            }
        }
    } else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<PPBCommunication *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< PPBState*>(_v) = _t->state(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *PPBCommunication::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PPBCommunication::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSPPBCommunicationENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return CommandInterface::qt_metacast(_clname);
}

int PPBCommunication::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CommandInterface::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 28)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 28;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 28)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 28;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void PPBCommunication::stateChanged(PPBState _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PPBCommunication::connected()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void PPBCommunication::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void PPBCommunication::busyChange(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void PPBCommunication::statusReceived(uint16_t _t1, const QVector<QByteArray> & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void PPBCommunication::commandProgress(int _t1, int _t2, TechCommand _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void PPBCommunication::commandCompleted(bool _t1, const QString & _t2, TechCommand _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void PPBCommunication::errorOccurred(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void PPBCommunication::logMessage(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void PPBCommunication::initialized()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}
QT_WARNING_POP
