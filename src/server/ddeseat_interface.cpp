/********************************************************************
Copyright 2020  wugang <wugang@uniontech.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include "ddeseat_interface.h"
#include "ddekeyboard_interface.h"
#include "global_p.h"
#include "resource_p.h"
#include "display.h"
#include "logging.h"

#include <QVector>
#include <wayland-server.h>
#include "wayland-dde-seat-server-protocol.h"

#define MAX_WINDOWS 50

namespace KWayland
{
namespace Server
{

class DDESeatInterface::Private : public Global::Private
{
public:

    Private(DDESeatInterface *q, Display *d);

    QVector<DDEPointerInterface*> ddePointers;
    static DDESeatInterface *get(wl_resource *native);

    QVector<DDEKeyboardInterface*> ddeKeyboards;
    quint32 timestamp = 0;

    QVector<DDETouchInterface*> ddeTouchs;
    quint32 touchtimestamp = 0;

    // Keyboard related members
    struct Keyboard {
        enum class State {
            Released,
            Pressed
        };
        QHash<quint32, State> states;
        struct Keymap {
            int fd = -1;
            quint32 size = 0;
            bool xkbcommonCompatible = false;
        };
        Keymap keymap;
        struct Modifiers {
            quint32 depressed = 0;
            quint32 latched = 0;
            quint32 locked = 0;
            quint32 group = 0;
            quint32 serial = 0;
        };
        Modifiers modifiers;
        quint32 lastStateSerial = 0;
        struct {
            qint32 charactersPerSecond = 0;
            qint32 delay = 0;
        } keyRepeat;
    };
    Keyboard keys;
    bool updateKey(quint32 key, Keyboard::State state);

private:
    void bind(wl_client *client, uint32_t version, uint32_t id) override;
    static Private *cast(wl_resource *r) {
        return reinterpret_cast<Private*>(wl_resource_get_user_data(r));
    }

    void getPointer(wl_client *client, wl_resource *resource, uint32_t id);
    void getKeyboard(wl_client *client, wl_resource *resource, uint32_t id);
    void getTouch(wl_client *client, wl_resource *resource, uint32_t id);
    // interface
    static void getPointerCallback(wl_client *client, wl_resource *resource, uint32_t id);
    static void getKeyboardCallback(wl_client *client, wl_resource *resource, uint32_t id);
    static void getTouchCallback(wl_client *client, wl_resource *resource, uint32_t id);
    static const struct dde_seat_interface s_interface;

    static const quint32 s_version;
    static const qint32 s_ddePointerVersion;
    static const qint32 s_ddeKeyboardVersion;
    static const qint32 s_ddeTouchVersion;
    DDESeatInterface *q;
};

class DDEPointerInterface::Private : public Resource::Private
{
public:
    Private(DDESeatInterface *parent, wl_resource *parentResource, DDEPointerInterface *q);

    DDESeatInterface *ddeSeat;
    QMetaObject::Connection destroyConnection;

private:
    DDEPointerInterface *q_func() {
        return reinterpret_cast<DDEPointerInterface *>(q);
    }

    void getMotion(wl_resource *resource);
    // interface
    static void getMotionCallback(wl_client *client, wl_resource *resource);

    static const struct dde_pointer_interface s_interface;
};

class DDETouchInterface::Private : public Resource::Private
{
public:
    Private(DDESeatInterface *parent, wl_resource *parentResource, DDETouchInterface *q);

    DDESeatInterface *ddeSeat;
    QMetaObject::Connection destroyConnection;

private:
    DDETouchInterface *q_func() {
        return reinterpret_cast<DDETouchInterface *>(q);
    }

    static const struct dde_touch_interface s_touch_interface;
};

const quint32 DDESeatInterface::Private::s_version = 1;
const qint32 DDESeatInterface::Private::s_ddePointerVersion = 1;
const qint32 DDESeatInterface::Private::s_ddeKeyboardVersion = 7;
const qint32 DDESeatInterface::Private::s_ddeTouchVersion = 1;

DDESeatInterface::Private::Private(DDESeatInterface *q, Display *d)
    : Global::Private(d, &dde_seat_interface, s_version)
    , q(q)
{
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct dde_seat_interface DDESeatInterface::Private::s_interface = {
    getPointerCallback,
    getKeyboardCallback,
    getTouchCallback,
};
#endif

DDESeatInterface *DDESeatInterface::Private::get(wl_resource *native)
{
    if (Private *p = cast(native)) {
        return p->q;
    }
    return nullptr;
}

DDESeatInterface::Private *DDESeatInterface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

DDESeatInterface::DDESeatInterface(Display *display, QObject *parent)
    : Global(new Private(this, display), parent)
{
}

DDESeatInterface::~DDESeatInterface() = default;

DDESeatInterface *DDESeatInterface::get(wl_resource* native)
{
    return Private::get(native);
}

void DDESeatInterface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    auto c = display->getConnection(client);
    wl_resource *resource = c->createResource(&dde_seat_interface, qMin(version, s_version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource, &s_interface, this, nullptr);

}

void DDESeatInterface::Private::getPointerCallback(wl_client *client, wl_resource *resource, uint32_t id)
{
    cast(resource)->getPointer(client, resource, id);
}

void DDESeatInterface::Private::getPointer(wl_client *client, wl_resource *resource, uint32_t id)
{
    // TODO: only create if seat has pointer?
    DDEPointerInterface *ddePointer = new DDEPointerInterface(q, resource);
    auto clientConnection = display->getConnection(client);
    ddePointer->create(clientConnection, qMin(wl_resource_get_version(resource), s_ddePointerVersion), id);
    if (!ddePointer->resource()) {
        wl_resource_post_no_memory(resource);
        delete ddePointer;
        return;
    }
    ddePointers << ddePointer;
    QObject::connect(ddePointer, &QObject::destroyed, q,
        [ddePointer,this] {
            ddePointers.removeAt(ddePointers.indexOf(ddePointer));
        }
    );
    Q_EMIT q->ddePointerCreated(ddePointer);
}

void DDESeatInterface::Private::getKeyboardCallback(wl_client *client, wl_resource *resource, uint32_t id)
{
    cast(resource)->getKeyboard(client, resource, id);
}

void DDESeatInterface::Private::getKeyboard(wl_client *client, wl_resource *resource, uint32_t id)
{
    // TODO: only create if seat has keyboard?
    DDEKeyboardInterface *keyboard = new DDEKeyboardInterface(q, resource);
    auto clientConnection = display->getConnection(client);
    keyboard->create(clientConnection, qMin(wl_resource_get_version(resource), s_ddeKeyboardVersion), id);
    if (!keyboard->resource()) {
        wl_resource_post_no_memory(resource);
        delete keyboard;
        return;
    }
    keyboard->repeatInfo(keys.keyRepeat.charactersPerSecond, keys.keyRepeat.delay);
    if (keys.keymap.xkbcommonCompatible) {
        keyboard->setKeymap(keys.keymap.fd, keys.keymap.size);
    }
    ddeKeyboards << keyboard;
    QObject::connect(keyboard, &QObject::destroyed, q,
        [keyboard,this] {
            ddeKeyboards.removeAt(ddeKeyboards.indexOf(keyboard));
        }
    );
    Q_EMIT q->ddeKeyboardCreated(keyboard);
}

void DDESeatInterface::Private::getTouchCallback(wl_client *client, wl_resource *resource, uint32_t id)
{
    cast(resource)->getTouch(client, resource, id);
}

void DDESeatInterface::Private::getTouch(wl_client *client, wl_resource *resource, uint32_t id)
{
    // TODO: only create if seat has touch?
    DDETouchInterface *touch = new DDETouchInterface(q, resource);
    auto clientConnection = display->getConnection(client);
    touch->create(clientConnection, qMin(wl_resource_get_version(resource), s_ddeTouchVersion), id);
    if (!touch->resource()) {
        wl_resource_post_no_memory(resource);
        delete touch;
        return;
    }
    ddeTouchs << touch;
    QObject::connect(touch, &QObject::destroyed, q,
        [touch,this] {
            ddeTouchs.removeAt(ddeTouchs.indexOf(touch));
        }
    );
    Q_EMIT q->ddeTouchCreated(touch);
}

bool DDESeatInterface::Private::updateKey(quint32 key, Keyboard::State state)
{
    auto it = keys.states.find(key);
    if (it == keys.states.end()) {
        keys.states.insert(key, state);
        return true;
    }
    if (it.value() == state) {
        return false;
    }
    it.value() = state;
    return true;
}

QPointF DDESeatInterface::pointerPos() const
{
    Q_D();
    return globalPos;
}

void DDESeatInterface::setPointerPos(const QPointF &pos)
{
    Q_D();
    if (globalPos == pos) {
        return;
    }
    globalPos = pos;
    Q_EMIT pointerPosChanged(pos);
}

void DDESeatInterface::pointerButtonPressed(quint32 button)
{
    Q_D();
    for (auto it = d->ddePointers.constBegin(), end = d->ddePointers.constEnd(); it != end; ++it) {
        (*it)->buttonPressed(button);
    }
}

void DDESeatInterface::pointerButtonReleased(quint32 button)
{
    Q_D();
    for (auto it = d->ddePointers.constBegin(), end = d->ddePointers.constEnd(); it != end; ++it) {
        (*it)->buttonReleased(button);
    }
}

void DDESeatInterface::pointerAxis(Qt::Orientation orientation, qint32 delta)
{
    Q_D();
    for (auto it = d->ddePointers.constBegin(), end = d->ddePointers.constEnd(); it != end; ++it) {
        (*it)->axis(orientation, delta);
    }
}

quint32 DDESeatInterface::timestamp() const
{
    Q_D();
    return d->timestamp;
}

void DDESeatInterface::setTimestamp(quint32 time)
{
    Q_D();
    if (d->timestamp == time) {
        return;
    }
    d->timestamp = time;
}

quint32 DDESeatInterface::touchtimestamp() const
{
    Q_D();
    return d->touchtimestamp;
}

void DDESeatInterface::setTouchTimestamp(quint32 time)
{
    Q_D();
    if (d->touchtimestamp == time) {
        return;
    }
    d->touchtimestamp = time;
}

void DDESeatInterface::setKeymap(int fd, quint32 size)
{
    Q_D();
    d->keys.keymap.xkbcommonCompatible = true;
    d->keys.keymap.fd = fd;
    d->keys.keymap.size = size;
    for (auto it = d->ddeKeyboards.constBegin(); it != d->ddeKeyboards.constEnd(); ++it) {
        (*it)->setKeymap(fd, size);
    }
}

void DDESeatInterface::keyPressed(quint32 key)
{
    Q_D();
    d->keys.lastStateSerial = d->display->nextSerial();
    if (!d->updateKey(key, Private::Keyboard::State::Pressed)) {
        return;
    }
    for (auto it = d->ddeKeyboards.constBegin(), end = d->ddeKeyboards.constEnd(); it != end; ++it) {
        (*it)->keyPressed(key, d->keys.lastStateSerial);
    }
}

void DDESeatInterface::keyReleased(quint32 key)
{
    Q_D();
    d->keys.lastStateSerial = d->display->nextSerial();
    if (!d->updateKey(key, Private::Keyboard::State::Released)) {
        return;
    }
    for (auto it = d->ddeKeyboards.constBegin(), end = d->ddeKeyboards.constEnd(); it != end; ++it) {
        (*it)->keyReleased(key, d->keys.lastStateSerial);
    }
}

void DDESeatInterface::touchDown(qint32 id, const QPointF &pos)
{
    Q_D();
    for (auto it = d->ddeTouchs.constBegin(), end = d->ddeTouchs.constEnd(); it != end; ++it) {
        (*it)->touchDown(id, pos);
    }
}

void DDESeatInterface::touchMotion(qint32 id, const QPointF &pos)
{
    Q_D();
    for (auto it = d->ddeTouchs.constBegin(), end = d->ddeTouchs.constEnd(); it != end; ++it) {
        (*it)->touchMotion(id, pos);
    }
}

void DDESeatInterface::touchUp(qint32 id)
{
    Q_D();
    for (auto it = d->ddeTouchs.constBegin(), end = d->ddeTouchs.constEnd(); it != end; ++it) {
        (*it)->touchUp(id);
    }
}

void DDESeatInterface::updateKeyboardModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group)
{
    Q_D();
    bool changed = false;
#define UPDATE( value ) \
    if (d->keys.modifiers.value != value) { \
        d->keys.modifiers.value = value; \
        changed = true; \
    }
    UPDATE(depressed)
    UPDATE(latched)
    UPDATE(locked)
    UPDATE(group)
    if (!changed) {
        return;
    }
    const quint32 serial = d->display->nextSerial();
    d->keys.modifiers.serial = serial;
    for (auto it = d->ddeKeyboards.constBegin(), end = d->ddeKeyboards.constEnd(); it != end; ++it) {
        (*it)->updateModifiers(depressed, latched, locked, group, serial);
    }
}

quint32 DDESeatInterface::depressedModifiers() const
{
    Q_D();
    return d->keys.modifiers.depressed;
}

quint32 DDESeatInterface::groupModifiers() const
{
    Q_D();
    return d->keys.modifiers.group;
}

quint32 DDESeatInterface::latchedModifiers() const
{
    Q_D();
    return d->keys.modifiers.latched;
}

quint32 DDESeatInterface::lockedModifiers() const
{
    Q_D();
    return d->keys.modifiers.locked;
}

quint32 DDESeatInterface::lastModifiersSerial() const
{
    Q_D();
    return d->keys.modifiers.serial;
}

/*********************************
 * DDEPointerInterface
 *********************************/
DDEPointerInterface::Private::Private(DDESeatInterface *parent, wl_resource *parentResource, DDEPointerInterface *q)
    : Resource::Private(q, parent, parentResource, &dde_pointer_interface, &s_interface)
    , ddeSeat(parent)
{
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct dde_pointer_interface DDEPointerInterface::Private::s_interface = {
    getMotionCallback,
};
#endif

DDEPointerInterface::DDEPointerInterface(DDESeatInterface *parent, wl_resource *parentResource)
    : Resource(new Private(parent, parentResource, this))
{
    connect(parent, &DDESeatInterface::pointerPosChanged, this, [this] {
        Q_D();
        if (d->resource) {
                const QPointF globalPos = d->ddeSeat->pointerPos();
                dde_pointer_send_motion(d->resource, wl_fixed_from_double(globalPos.x()), wl_fixed_from_double(globalPos.y()));
        }
    });
}

DDEPointerInterface::~DDEPointerInterface() = default;

DDESeatInterface *DDEPointerInterface::ddeSeat() const {
    Q_D();
    return reinterpret_cast<DDESeatInterface*>(d->global);
}

DDEPointerInterface::Private *DDEPointerInterface::d_func() const
{
    return reinterpret_cast<DDEPointerInterface::Private*>(d.data());
}

void DDEPointerInterface::Private::getMotion(wl_resource *resource)
{
    const QPointF globalPos = ddeSeat->pointerPos();
    dde_pointer_send_motion(resource, wl_fixed_from_double(globalPos.x()), wl_fixed_from_double(globalPos.y()));
}

void DDEPointerInterface::Private::getMotionCallback(wl_client *client, wl_resource *resource)
{
    auto s = cast<Private>(resource);
    Q_ASSERT(client == *s->client);

    s->getMotion(resource);
}

void DDEPointerInterface::buttonPressed(quint32 button)
{
    Q_D();
    if (!d->resource) {
        return;
    }
    const QPointF globalPos = d->ddeSeat->pointerPos();
    dde_pointer_send_button(d->resource, wl_fixed_from_double(globalPos.x()), wl_fixed_from_double(globalPos.y()), button, DDE_POINTER_BUTTON_STATE_PRESSED);
}

void DDEPointerInterface::buttonReleased(quint32 button)
{
    Q_D();
    if (!d->resource) {
        return;
    }
    const QPointF globalPos = d->ddeSeat->pointerPos();
    dde_pointer_send_button(d->resource, wl_fixed_from_double(globalPos.x()), wl_fixed_from_double(globalPos.y()), button, DDE_POINTER_BUTTON_STATE_RELEASED);
}

void DDEPointerInterface::axis(Qt::Orientation orientation, qint32 delta)
{
    Q_D();
    if (!d->resource) {
        return;
    }
    dde_pointer_send_axis(d->resource, 0,
                         (orientation == Qt::Vertical) ? WL_POINTER_AXIS_VERTICAL_SCROLL : WL_POINTER_AXIS_HORIZONTAL_SCROLL,
                         wl_fixed_from_int(delta));
}

/*********************************
 * DDETouchInterface
 *********************************/
DDETouchInterface::Private::Private(DDESeatInterface *parent, wl_resource *parentResource, DDETouchInterface *q)
    : Resource::Private(q, parent, parentResource, &dde_touch_interface, &s_touch_interface)
    , ddeSeat(parent)
{
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct dde_touch_interface DDETouchInterface::Private::s_touch_interface = {
    resourceDestroyedCallback,
};
#endif

DDETouchInterface::DDETouchInterface(DDESeatInterface *parent, wl_resource *parentResource)
    : Resource(new Private(parent, parentResource, this))
{
}

DDETouchInterface::~DDETouchInterface() = default;

DDESeatInterface *DDETouchInterface::ddeSeat() const {
    Q_D();
    return reinterpret_cast<DDESeatInterface*>(d->global);
}

DDETouchInterface::Private *DDETouchInterface::d_func() const
{
    return reinterpret_cast<DDETouchInterface::Private*>(d.data());
}

void DDETouchInterface::touchDown(qint32 id, const QPointF &pos)
{
    Q_D();
    if (!d->resource) {
        return;
    }

    dde_touch_send_down(d->resource, id, d->ddeSeat->touchtimestamp(), wl_fixed_from_double(pos.x()), wl_fixed_from_double(pos.y()));
}

void DDETouchInterface::touchMotion(qint32 id, const QPointF &pos)
{
    Q_D();
    if (!d->resource) {
        return;
    }

    dde_touch_send_motion(d->resource, id, d->ddeSeat->touchtimestamp(), wl_fixed_from_double(pos.x()), wl_fixed_from_double(pos.y()));
}

void DDETouchInterface::touchUp(qint32 id)
{
    Q_D();
    if (!d->resource) {
        return;
    }

    dde_touch_send_up(d->resource, id, d->ddeSeat->touchtimestamp());
}

}
}