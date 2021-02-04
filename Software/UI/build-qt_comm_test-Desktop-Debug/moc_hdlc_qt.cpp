/****************************************************************************
** Meta object code from reading C++ file 'hdlc_qt.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../qt_comm_test/hdlc_qt.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'hdlc_qt.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_HDLC_qt_t {
    QByteArrayData data[12];
    char stringdata0[142];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_HDLC_qt_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_HDLC_qt_t qt_meta_stringdata_HDLC_qt = {
    {
QT_MOC_LITERAL(0, 0, 7), // "HDLC_qt"
QT_MOC_LITERAL(1, 8, 16), // "hdlcTransmitByte"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 4), // "data"
QT_MOC_LITERAL(4, 31, 22), // "hdlcValidFrameReceived"
QT_MOC_LITERAL(5, 54, 20), // "receive_frame_buffer"
QT_MOC_LITERAL(6, 75, 10), // "frame_size"
QT_MOC_LITERAL(7, 86, 12), // "charReceiver"
QT_MOC_LITERAL(8, 99, 9), // "dataArray"
QT_MOC_LITERAL(9, 109, 11), // "frameDecode"
QT_MOC_LITERAL(10, 121, 6), // "buffer"
QT_MOC_LITERAL(11, 128, 13) // "bytes_to_send"

    },
    "HDLC_qt\0hdlcTransmitByte\0\0data\0"
    "hdlcValidFrameReceived\0receive_frame_buffer\0"
    "frame_size\0charReceiver\0dataArray\0"
    "frameDecode\0buffer\0bytes_to_send"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_HDLC_qt[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x06 /* Public */,
       1,    1,   42,    2, 0x06 /* Public */,
       4,    2,   45,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    1,   50,    2, 0x0a /* Public */,
       9,    2,   53,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QByteArray,    3,
    QMetaType::Void, QMetaType::Char,    3,
    QMetaType::Void, QMetaType::QByteArray, QMetaType::UShort,    5,    6,

 // slots: parameters
    QMetaType::Void, QMetaType::QByteArray,    8,
    QMetaType::Void, QMetaType::QByteArray, QMetaType::UShort,   10,   11,

       0        // eod
};

void HDLC_qt::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<HDLC_qt *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->hdlcTransmitByte((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        case 1: _t->hdlcTransmitByte((*reinterpret_cast< char(*)>(_a[1]))); break;
        case 2: _t->hdlcValidFrameReceived((*reinterpret_cast< QByteArray(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2]))); break;
        case 3: _t->charReceiver((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        case 4: _t->frameDecode((*reinterpret_cast< QByteArray(*)>(_a[1])),(*reinterpret_cast< quint16(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (HDLC_qt::*)(QByteArray );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&HDLC_qt::hdlcTransmitByte)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (HDLC_qt::*)(char );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&HDLC_qt::hdlcTransmitByte)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (HDLC_qt::*)(QByteArray , quint16 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&HDLC_qt::hdlcValidFrameReceived)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject HDLC_qt::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_HDLC_qt.data,
    qt_meta_data_HDLC_qt,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *HDLC_qt::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *HDLC_qt::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_HDLC_qt.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int HDLC_qt::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void HDLC_qt::hdlcTransmitByte(QByteArray _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void HDLC_qt::hdlcTransmitByte(char _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void HDLC_qt::hdlcValidFrameReceived(QByteArray _t1, quint16 _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
