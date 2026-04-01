/****************************************************************************
** Meta object code from reading C++ file 'QtUiSpikeState.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/qtui/QtUiSpikeState.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QtUiSpikeState.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.0. It"
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
struct qt_meta_tag_ZN14QtUiSpikeStateE_t {};
} // unnamed namespace

template <> constexpr inline auto QtUiSpikeState::qt_create_metaobjectdata<qt_meta_tag_ZN14QtUiSpikeStateE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "QtUiSpikeState",
        "backendNameChanged",
        "",
        "modeNameChanged",
        "architectureNoteChanged",
        "loginStatusChanged",
        "chatPreviewChanged",
        "lastInputChanged",
        "anchorsChanged",
        "backendName",
        "modeName",
        "architectureNote",
        "loginStatus",
        "chatPreview",
        "lastInput",
        "anchors",
        "QVariantList"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'backendNameChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'modeNameChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'architectureNoteChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'loginStatusChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'chatPreviewChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'lastInputChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'anchorsChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'backendName'
        QtMocHelpers::PropertyData<QString>(9, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'modeName'
        QtMocHelpers::PropertyData<QString>(10, QMetaType::QString, QMC::DefaultPropertyFlags, 1),
        // property 'architectureNote'
        QtMocHelpers::PropertyData<QString>(11, QMetaType::QString, QMC::DefaultPropertyFlags, 2),
        // property 'loginStatus'
        QtMocHelpers::PropertyData<QString>(12, QMetaType::QString, QMC::DefaultPropertyFlags, 3),
        // property 'chatPreview'
        QtMocHelpers::PropertyData<QString>(13, QMetaType::QString, QMC::DefaultPropertyFlags, 4),
        // property 'lastInput'
        QtMocHelpers::PropertyData<QString>(14, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
        // property 'anchors'
        QtMocHelpers::PropertyData<QVariantList>(15, 0x80000000 | 16, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 6),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<QtUiSpikeState, qt_meta_tag_ZN14QtUiSpikeStateE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject QtUiSpikeState::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14QtUiSpikeStateE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14QtUiSpikeStateE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14QtUiSpikeStateE_t>.metaTypes,
    nullptr
} };

void QtUiSpikeState::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<QtUiSpikeState *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->backendNameChanged(); break;
        case 1: _t->modeNameChanged(); break;
        case 2: _t->architectureNoteChanged(); break;
        case 3: _t->loginStatusChanged(); break;
        case 4: _t->chatPreviewChanged(); break;
        case 5: _t->lastInputChanged(); break;
        case 6: _t->anchorsChanged(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (QtUiSpikeState::*)()>(_a, &QtUiSpikeState::backendNameChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (QtUiSpikeState::*)()>(_a, &QtUiSpikeState::modeNameChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (QtUiSpikeState::*)()>(_a, &QtUiSpikeState::architectureNoteChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (QtUiSpikeState::*)()>(_a, &QtUiSpikeState::loginStatusChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (QtUiSpikeState::*)()>(_a, &QtUiSpikeState::chatPreviewChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (QtUiSpikeState::*)()>(_a, &QtUiSpikeState::lastInputChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (QtUiSpikeState::*)()>(_a, &QtUiSpikeState::anchorsChanged, 6))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QString*>(_v) = _t->backendName(); break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->modeName(); break;
        case 2: *reinterpret_cast<QString*>(_v) = _t->architectureNote(); break;
        case 3: *reinterpret_cast<QString*>(_v) = _t->loginStatus(); break;
        case 4: *reinterpret_cast<QString*>(_v) = _t->chatPreview(); break;
        case 5: *reinterpret_cast<QString*>(_v) = _t->lastInput(); break;
        case 6: *reinterpret_cast<QVariantList*>(_v) = _t->anchors(); break;
        default: break;
        }
    }
}

const QMetaObject *QtUiSpikeState::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QtUiSpikeState::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14QtUiSpikeStateE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int QtUiSpikeState::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void QtUiSpikeState::backendNameChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void QtUiSpikeState::modeNameChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void QtUiSpikeState::architectureNoteChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void QtUiSpikeState::loginStatusChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void QtUiSpikeState::chatPreviewChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void QtUiSpikeState::lastInputChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void QtUiSpikeState::anchorsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}
QT_WARNING_POP
