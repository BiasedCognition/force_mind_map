/****************************************************************************
** Meta object code from reading C++ file 'WebNodeManager.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../include/WebNodeManager.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'WebNodeManager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.1. It"
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
struct qt_meta_tag_ZN14WebNodeManagerE_t {};
} // unnamed namespace

template <> constexpr inline auto WebNodeManager::qt_create_metaobjectdata<qt_meta_tag_ZN14WebNodeManagerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "WebNodeManager",
        "nodeAdded",
        "",
        "node",
        "nodeRemoved",
        "id",
        "nodeUpdated",
        "connectionAdded",
        "link",
        "connectionRemoved",
        "sourceId",
        "targetId",
        "graphUpdated",
        "fullGraph",
        "addNode",
        "text",
        "parentId",
        "removeNode",
        "updateNodeText",
        "newText",
        "addConnection",
        "removeConnection",
        "getFullGraph"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'nodeAdded'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 3 },
        }}),
        // Signal 'nodeRemoved'
        QtMocHelpers::SignalData<void(const QString &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 },
        }}),
        // Signal 'nodeUpdated'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 3 },
        }}),
        // Signal 'connectionAdded'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 8 },
        }}),
        // Signal 'connectionRemoved'
        QtMocHelpers::SignalData<void(const QString &, const QString &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 }, { QMetaType::QString, 11 },
        }}),
        // Signal 'graphUpdated'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 13 },
        }}),
        // Method 'addNode'
        QtMocHelpers::MethodData<void(const QString &, const QString &, const QString &)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 }, { QMetaType::QString, 15 }, { QMetaType::QString, 16 },
        }}),
        // Method 'addNode'
        QtMocHelpers::MethodData<void(const QString &, const QString &)>(14, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::QString, 5 }, { QMetaType::QString, 15 },
        }}),
        // Method 'removeNode'
        QtMocHelpers::MethodData<void(const QString &)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 },
        }}),
        // Method 'updateNodeText'
        QtMocHelpers::MethodData<void(const QString &, const QString &)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 }, { QMetaType::QString, 19 },
        }}),
        // Method 'addConnection'
        QtMocHelpers::MethodData<void(const QString &, const QString &)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 }, { QMetaType::QString, 11 },
        }}),
        // Method 'removeConnection'
        QtMocHelpers::MethodData<void(const QString &, const QString &)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 }, { QMetaType::QString, 11 },
        }}),
        // Method 'getFullGraph'
        QtMocHelpers::MethodData<QJsonObject() const>(22, 2, QMC::AccessPublic, QMetaType::QJsonObject),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<WebNodeManager, qt_meta_tag_ZN14WebNodeManagerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject WebNodeManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14WebNodeManagerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14WebNodeManagerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14WebNodeManagerE_t>.metaTypes,
    nullptr
} };

void WebNodeManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<WebNodeManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->nodeAdded((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 1: _t->nodeRemoved((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->nodeUpdated((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 3: _t->connectionAdded((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 4: _t->connectionRemoved((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 5: _t->graphUpdated((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 6: _t->addNode((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 7: _t->addNode((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 8: _t->removeNode((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 9: _t->updateNodeText((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 10: _t->addConnection((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 11: _t->removeConnection((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 12: { QJsonObject _r = _t->getFullGraph();
            if (_a[0]) *reinterpret_cast< QJsonObject*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (WebNodeManager::*)(const QJsonObject & )>(_a, &WebNodeManager::nodeAdded, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (WebNodeManager::*)(const QString & )>(_a, &WebNodeManager::nodeRemoved, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (WebNodeManager::*)(const QJsonObject & )>(_a, &WebNodeManager::nodeUpdated, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (WebNodeManager::*)(const QJsonObject & )>(_a, &WebNodeManager::connectionAdded, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (WebNodeManager::*)(const QString & , const QString & )>(_a, &WebNodeManager::connectionRemoved, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (WebNodeManager::*)(const QJsonObject & )>(_a, &WebNodeManager::graphUpdated, 5))
            return;
    }
}

const QMetaObject *WebNodeManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WebNodeManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14WebNodeManagerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int WebNodeManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void WebNodeManager::nodeAdded(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void WebNodeManager::nodeRemoved(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void WebNodeManager::nodeUpdated(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void WebNodeManager::connectionAdded(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void WebNodeManager::connectionRemoved(const QString & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2);
}

// SIGNAL 5
void WebNodeManager::graphUpdated(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}
QT_WARNING_POP
