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
#include "ddeseat.h"
#include "event_queue.h"
#include "logging.h"
#include "wayland_pointer_p.h"
// Qt
#include <QDebug>
#include <QVector>
#include <QPointF>
#include <QPointer>
// wayland
#include "wayland-dde-seat-client-protocol.h"
#include <wayland-client-protocol.h>

namespace KWayland
{

namespace Client
{

class Q_DECL_HIDDEN DDESeat::Private
{
public:
    Private(DDESeat *q);
    void setup(dde_seat *o);

    WaylandPointer<dde_seat, dde_seat_destroy> ddeSeat;
    EventQueue *queue = nullptr;

private:
    DDESeat *q;
};

DDESeat::Private::Private(DDESeat *q)
    : q(q)
{
}

void DDESeat::Private::setup(dde_seat *o)
{
    Q_ASSERT(o);
    Q_ASSERT(!ddeSeat);
    ddeSeat.setup(o);
}

DDESeat::DDESeat(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

DDESeat::~DDESeat()
{
    release();
}

void DDESeat::destroy()
{
    if (!d->ddeSeat) {
        return;
    }
    emit interfaceAboutToBeDestroyed();
    d->ddeSeat.destroy();
}

void DDESeat::release()
{
    if (!d->ddeSeat) {
        return;
    }
    emit interfaceAboutToBeReleased();
    d->ddeSeat.release();
}

void DDESeat::setup(dde_seat *ddeSeat)
{
    d->setup(ddeSeat);
}

EventQueue *DDESeat::eventQueue() const
{
    return d->queue;
}

void DDESeat::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

DDEPointer *DDESeat::createDDePointer(QObject *parent)
{
    Q_ASSERT(isValid());

    DDEPointer *s = new DDEPointer(parent);
    connect(this, &DDESeat::interfaceAboutToBeReleased, s, &DDEPointer::release);
    connect(this, &DDESeat::interfaceAboutToBeDestroyed, s, &DDEPointer::destroy);
    auto w = dde_seat_get_dde_pointer(d->ddeSeat);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

bool DDESeat::isValid() const
{
    return d->ddeSeat.isValid();
}

DDESeat::operator dde_seat*() {
    return d->ddeSeat;
}

DDESeat::operator dde_seat*() const {
    return d->ddeSeat;
}


class Q_DECL_HIDDEN DDEPointer::Private
{
public:
    Private(DDEPointer *q);
    void setup(dde_pointer *p);

    WaylandPointer<dde_pointer, dde_pointer_destroy> ddePointer;
    QPointF globalPointerPos;
private:
    static void motionCallback(void *data, dde_pointer *ddePointer, wl_fixed_t sx, wl_fixed_t sy);
    static void buttonCallback(void *data, dde_pointer *ddePointer, wl_fixed_t sx, wl_fixed_t sy, uint32_t button, uint32_t state);

    DDEPointer *q;
    static const dde_pointer_listener s_listener;
};

DDEPointer::Private::Private(DDEPointer *q)
    : q(q)
{
}

void DDEPointer::Private::setup(dde_pointer *p)
{
    Q_ASSERT(p);
    Q_ASSERT(!ddePointer);
    ddePointer.setup(p);
    dde_pointer_add_listener(ddePointer, &s_listener, this);
}

const dde_pointer_listener DDEPointer::Private::s_listener = {
    motionCallback,
    buttonCallback,
};

DDEPointer::DDEPointer(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

DDEPointer::~DDEPointer()
{
    release();
}

void DDEPointer::release()
{
    d->ddePointer.release();
}

void DDEPointer::destroy()
{
    d->ddePointer.destroy();
}

void DDEPointer::setup(dde_pointer *ddePointer)
{
    d->setup(ddePointer);
}

void DDEPointer::Private::motionCallback(void *data, dde_pointer *ddePointer, wl_fixed_t sx, wl_fixed_t sy)
{
    auto p = reinterpret_cast<DDEPointer::Private*>(data);
    Q_ASSERT(p->ddePointer == ddePointer);
    p->globalPointerPos = QPointF(wl_fixed_to_double(sx), wl_fixed_to_double(sy));
    emit p->q->motion(p->globalPointerPos);
}

void DDEPointer::Private::buttonCallback(void *data, dde_pointer *ddePointer, wl_fixed_t sx, wl_fixed_t sy, uint32_t button, uint32_t state)
{
    auto p = reinterpret_cast<DDEPointer::Private*>(data);
    Q_ASSERT(p->ddePointer == ddePointer);
    auto toState = [state] {
        if (state == DDE_POINTER_BUTTON_STATE_RELEASED) {
            return ButtonState::Released;
        } else {
            return ButtonState::Pressed;
        }
    };
    p->globalPointerPos = QPointF(wl_fixed_to_double(sx), wl_fixed_to_double(sy));
    emit p->q->buttonStateChanged(p->globalPointerPos, button, toState());
}

bool DDEPointer::isValid() const
{
    return d->ddePointer.isValid();
}

DDEPointer::operator dde_pointer*() const
{
    return d->ddePointer;
}

DDEPointer::operator dde_pointer*()
{
    return d->ddePointer;
}

void DDEPointer::getMotion()
{
    dde_pointer_get_motion(d->ddePointer);
}

QPointF DDEPointer::getGlobalPointerPos()
{
    return d->globalPointerPos;
}


}
}
