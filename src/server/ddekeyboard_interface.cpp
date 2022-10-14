// Copyright 2021  luochaojiang <luochaojiang@uniontech.com>
// SPDX-FileCopyrightText: 2022 Martin Gräßlin <mgraesslin@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "ddeseat_interface.h"
#include "ddekeyboard_interface.h"
#include "global_p.h"
#include "resource_p.h"
#include "display.h"
#include "logging.h"

#include <QVector>
#include <wayland-server.h>
#include "wayland-dde-seat-server-protocol.h"

namespace KWayland
{
namespace Server
{

class DDEKeyboardInterface::Private : public Resource::Private
{
public:
    Private(DDESeatInterface *parent, wl_resource *parentResource, DDEKeyboardInterface *q);

    void sendKeymap(int fd, quint32 size);
    void sendModifiers();
    void sendModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group, quint32 serial);

    DDESeatInterface *ddeSeat;
    QMetaObject::Connection destroyConnection;

private:
    DDEKeyboardInterface *q_func() {
        return reinterpret_cast<DDEKeyboardInterface *>(q);
    }

    static const struct dde_keyboard_interface s_keboard_interface;
};

/*********************************
 * DDEKeyboardInterface
 *********************************/
DDEKeyboardInterface::Private::Private(DDESeatInterface *parent, wl_resource *parentResource, DDEKeyboardInterface *q)
    : Resource::Private(q, parent, parentResource, &dde_keyboard_interface, &s_keboard_interface)
    , ddeSeat(parent)
{
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct dde_keyboard_interface DDEKeyboardInterface::Private::s_keboard_interface = {
    resourceDestroyedCallback,
};
#endif

DDEKeyboardInterface::DDEKeyboardInterface(DDESeatInterface *parent, wl_resource *parentResource)
    : Resource(new Private(parent, parentResource, this))
{
}

DDEKeyboardInterface::~DDEKeyboardInterface() = default;

DDESeatInterface *DDEKeyboardInterface::ddeSeat() const {
    Q_D();
    return reinterpret_cast<DDESeatInterface*>(d->global);
}

DDEKeyboardInterface::Private *DDEKeyboardInterface::d_func() const
{
    return reinterpret_cast<DDEKeyboardInterface::Private*>(d.data());
}

void DDEKeyboardInterface::setKeymap(int fd, quint32 size)
{
    Q_D();
    d->sendKeymap(fd, size);
}

void DDEKeyboardInterface::Private::sendKeymap(int fd, quint32 size)
{
    if (!resource) {
        return;
    }
    dde_keyboard_send_keymap(resource, DDE_KEYBOARD_KEYMAP_FORMAT_XKB_V1, fd, size);
}

void DDEKeyboardInterface::Private::sendModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group, quint32 serial)
{
    if (!resource) {
        return;
    }
    dde_keyboard_send_modifiers(resource, serial, depressed, latched, locked, group);
}

void DDEKeyboardInterface::Private::sendModifiers()
{
    sendModifiers(ddeSeat->depressedModifiers(), ddeSeat->latchedModifiers(), ddeSeat->lockedModifiers(), ddeSeat->groupModifiers(), ddeSeat->lastModifiersSerial());
}

void DDEKeyboardInterface::keyPressed(quint32 key, quint32 serial)
{
    Q_D();
    if (!d->resource) {
        return;
    }

    dde_keyboard_send_key(d->resource, serial, d->ddeSeat->timestamp(), key, DDE_KEYBOARD_KEY_STATE_PRESSED);
}

void DDEKeyboardInterface::keyReleased(quint32 key, quint32 serial)
{
    Q_D();
    if (!d->resource) {
        return;
    }

    dde_keyboard_send_key(d->resource, serial, d->ddeSeat->timestamp(), key, DDE_KEYBOARD_KEY_STATE_RELEASED);
}

void DDEKeyboardInterface::updateModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group, quint32 serial)
{
    Q_D();
    d->sendModifiers(depressed, latched, locked, group, serial);
}

void DDEKeyboardInterface::repeatInfo(qint32 charactersPerSecond, qint32 delay)
{
    Q_D();
    if (!d->resource) {
        return;
    }
    if (wl_resource_get_version(d->resource) < DDE_KEYBOARD_REPEAT_INFO_SINCE_VERSION) {
        // only supported since version 4
        return;
    }
    dde_keyboard_send_repeat_info(d->resource, charactersPerSecond, delay);
}

}
}