/********************************************************************
Copyright 2021  luochaojiang <luochaojiang@uniontech.com>

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
#include "ddeseat.h"
#include "ddekeyboard.h"
#include "wayland_pointer_p.h"
// wayland
#include "wayland-dde-seat-client-protocol.h"
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{

class Q_DECL_HIDDEN DDEKeyboard::Private
{
public:
    Private(DDEKeyboard *q);
    void setup(dde_keyboard *k);

    WaylandPointer<dde_keyboard, dde_keyboard_release> ddeKeyboard;

    struct {
        qint32 charactersPerSecond = 0;
        qint32 delay = 0;
    } repeatInfo;
private:
    static void keymapCallback(void *data, dde_keyboard *keyboard, uint32_t format, int fd, uint32_t size);
    static void enterCallback(void *data, dde_keyboard *keyboard, uint32_t serial, wl_surface *surface, wl_array *keys);
    static void leaveCallback(void *data, dde_keyboard *keyboard, uint32_t serial, wl_surface *surface);
    static void keyCallback(void *data, dde_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    static void modifiersCallback(void *data, dde_keyboard *keyboard, uint32_t serial, uint32_t modsDepressed,
                                  uint32_t modsLatched, uint32_t modsLocked, uint32_t group);
    static void repeatInfoCallback(void *data, dde_keyboard *keyboard, int32_t charactersPerSecond, int32_t delay);
    DDEKeyboard *q;
    static const dde_keyboard_listener s_listener;
};

DDEKeyboard::Private::Private(DDEKeyboard *q)
    : q(q)
{
}

void DDEKeyboard::Private::setup(dde_keyboard *k)
{
    Q_ASSERT(k);
    Q_ASSERT(!ddeKeyboard);
    ddeKeyboard.setup(k);
    dde_keyboard_add_listener(ddeKeyboard, &s_listener, this);
}

const dde_keyboard_listener DDEKeyboard::Private::s_listener = {
    keymapCallback,
    enterCallback,
    leaveCallback,
    keyCallback,
    modifiersCallback,
    repeatInfoCallback,
};

DDEKeyboard::DDEKeyboard(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

DDEKeyboard::~DDEKeyboard()
{
    release();
}

void DDEKeyboard::release()
{
    d->ddeKeyboard.release();
}

void DDEKeyboard::destroy()
{
    d->ddeKeyboard.destroy();
}

void DDEKeyboard::setup(dde_keyboard *keyboard)
{
    d->setup(keyboard);
}

bool DDEKeyboard::isValid() const
{
    return d->ddeKeyboard.isValid();
}

DDEKeyboard::operator dde_keyboard*()
{
    return d->ddeKeyboard;
}

DDEKeyboard::operator dde_keyboard*() const
{
    return d->ddeKeyboard;
}

void DDEKeyboard::Private::enterCallback(void *data, dde_keyboard *keyboard, uint32_t serial, wl_surface *surface, wl_array *keys)
{
    Q_UNUSED(data)
    Q_UNUSED(keyboard)
    Q_UNUSED(serial)
    Q_UNUSED(surface)
    Q_UNUSED(keys)
}

void DDEKeyboard::Private::leaveCallback(void *data, dde_keyboard *keyboard, uint32_t serial, wl_surface *surface)
{
    Q_UNUSED(data)
    Q_UNUSED(keyboard)
    Q_UNUSED(serial)
    Q_UNUSED(surface)
}

void DDEKeyboard::Private::keyCallback(void *data, dde_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    Q_UNUSED(serial)
    auto k = reinterpret_cast<DDEKeyboard::Private*>(data);
    Q_ASSERT(k->ddeKeyboard == keyboard);
    auto toState = [state] {
        if (state == DDE_KEYBOARD_KEY_STATE_RELEASED) {
            return KeyState::Released;
        } else {
            return KeyState::Pressed;
        }
    };
    Q_EMIT k->q->keyChanged(key, toState(), time);
}

void DDEKeyboard::Private::keymapCallback(void *data, dde_keyboard *keyboard, uint32_t format, int fd, uint32_t size)
{
    auto k = reinterpret_cast<DDEKeyboard::Private*>(data);
    Q_ASSERT(k->ddeKeyboard == keyboard);
    if (format != DDE_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        return;
    }
    Q_EMIT k->q->keymapChanged(fd, size);
}

void DDEKeyboard::Private::modifiersCallback(void *data, dde_keyboard *keyboard, uint32_t serial, uint32_t modsDepressed,
                                 uint32_t modsLatched, uint32_t modsLocked, uint32_t group)
{
    Q_UNUSED(serial)
    auto k = reinterpret_cast<DDEKeyboard::Private*>(data);
    Q_ASSERT(k->ddeKeyboard == keyboard);
    Q_EMIT k->q->modifiersChanged(modsDepressed, modsLatched, modsLocked, group);
}

void DDEKeyboard::Private::repeatInfoCallback(void *data, dde_keyboard *keyboard, int32_t charactersPerSecond, int32_t delay)
{
    auto k = reinterpret_cast<DDEKeyboard::Private*>(data);
    Q_ASSERT(k->ddeKeyboard == keyboard);
    k->repeatInfo.charactersPerSecond = qMax(charactersPerSecond, 0);
    k->repeatInfo.delay = qMax(delay, 0);
    Q_EMIT k->q->keyRepeatChanged();
}

}
}