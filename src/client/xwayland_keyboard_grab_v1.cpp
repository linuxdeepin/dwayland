// Copyright 2021  zhangyaning <zhangyaning@uniontech.com>
// SPDX-FileCopyrightText: 2022 Martin Gräßlin <mgraesslin@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "xwayland_keyboard_grab_v1.h"
#include "event_queue.h"
#include "surface.h"
#include "seat.h"
#include "logging.h"
#include "wayland_pointer_p.h"
// Qt
#include <QDebug>
#include <QVector>
#include <QPointF>
#include <QPointer>
// wayland
#include <wayland-xwayland-keyboard-grab-v1-client-protocol.h>

namespace KWayland
{

namespace Client
{

class Q_DECL_HIDDEN ZWPXwaylandKeyboardGrabManagerV1::Private
{
public:
    Private(ZWPXwaylandKeyboardGrabManagerV1 *q);
    void setup(zwp_xwayland_keyboard_grab_manager_v1 *o);

    WaylandPointer<zwp_xwayland_keyboard_grab_manager_v1, zwp_xwayland_keyboard_grab_manager_v1_destroy> grabManager;
    EventQueue *queue = nullptr;

private:
    ZWPXwaylandKeyboardGrabManagerV1 *q;
};

ZWPXwaylandKeyboardGrabManagerV1::Private::Private(ZWPXwaylandKeyboardGrabManagerV1 *q)
    : q(q)
{
}

void ZWPXwaylandKeyboardGrabManagerV1::Private::setup(zwp_xwayland_keyboard_grab_manager_v1 *o)
{
    Q_ASSERT(o);
    Q_ASSERT(!grabManager);
    grabManager.setup(o);
}

ZWPXwaylandKeyboardGrabManagerV1::ZWPXwaylandKeyboardGrabManagerV1(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

ZWPXwaylandKeyboardGrabManagerV1::~ZWPXwaylandKeyboardGrabManagerV1()
{
    release();
}

void ZWPXwaylandKeyboardGrabManagerV1::destroy()
{
    if (!d->grabManager) {
        return;
    }
    emit interfaceAboutToBeDestroyed();
    d->grabManager.destroy();
}

void ZWPXwaylandKeyboardGrabManagerV1::release()
{
    if (!d->grabManager) {
        return;
    }
    emit interfaceAboutToBeReleased();
    d->grabManager.release();
}

void ZWPXwaylandKeyboardGrabManagerV1::setup(zwp_xwayland_keyboard_grab_manager_v1 *grabManager)
{
    d->setup(grabManager);
}

EventQueue *ZWPXwaylandKeyboardGrabManagerV1::eventQueue() const
{
    return d->queue;
}

void ZWPXwaylandKeyboardGrabManagerV1::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

ZWPXwaylandKeyboardGrabV1 *ZWPXwaylandKeyboardGrabManagerV1::grabKeyBoard(Surface *surface, Seat *seat, QObject *parent)
{
    Q_ASSERT(isValid());

    ZWPXwaylandKeyboardGrabV1 *s = new ZWPXwaylandKeyboardGrabV1(parent);
    connect(this, &ZWPXwaylandKeyboardGrabManagerV1::interfaceAboutToBeReleased, s, &ZWPXwaylandKeyboardGrabV1::release);
    connect(this, &ZWPXwaylandKeyboardGrabManagerV1::interfaceAboutToBeDestroyed, s, &ZWPXwaylandKeyboardGrabV1::destroy);
    auto w = zwp_xwayland_keyboard_grab_manager_v1_grab_keyboard(d->grabManager, *surface, *seat);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

bool ZWPXwaylandKeyboardGrabManagerV1::isValid() const
{
    return d->grabManager.isValid();
}

ZWPXwaylandKeyboardGrabManagerV1::operator zwp_xwayland_keyboard_grab_manager_v1*() {
    return d->grabManager;
}

ZWPXwaylandKeyboardGrabManagerV1::operator zwp_xwayland_keyboard_grab_manager_v1*() const {
    return d->grabManager;
}


class Q_DECL_HIDDEN ZWPXwaylandKeyboardGrabV1::Private
{
public:
    Private(ZWPXwaylandKeyboardGrabV1 *q);
    void setup(zwp_xwayland_keyboard_grab_v1 *p);

    WaylandPointer<zwp_xwayland_keyboard_grab_v1, zwp_xwayland_keyboard_grab_v1_destroy> grab;
private:

    ZWPXwaylandKeyboardGrabV1 *q;
};

ZWPXwaylandKeyboardGrabV1::Private::Private(ZWPXwaylandKeyboardGrabV1 *q)
    : q(q)
{
}

void ZWPXwaylandKeyboardGrabV1::Private::setup(zwp_xwayland_keyboard_grab_v1 *p)
{
    Q_ASSERT(p);
    Q_ASSERT(!grab);
    grab.setup(p);
}

ZWPXwaylandKeyboardGrabV1::ZWPXwaylandKeyboardGrabV1(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

ZWPXwaylandKeyboardGrabV1::~ZWPXwaylandKeyboardGrabV1()
{
    release();
}

void ZWPXwaylandKeyboardGrabV1::release()
{
    d->grab.release();
}

void ZWPXwaylandKeyboardGrabV1::destroy()
{
    d->grab.destroy();
}

void ZWPXwaylandKeyboardGrabV1::setup(zwp_xwayland_keyboard_grab_v1 *grab)
{
    d->setup(grab);
}

bool ZWPXwaylandKeyboardGrabV1::isValid() const
{
    return d->grab.isValid();
}

ZWPXwaylandKeyboardGrabV1::operator zwp_xwayland_keyboard_grab_v1*() const
{
    return d->grab;
}

ZWPXwaylandKeyboardGrabV1::operator zwp_xwayland_keyboard_grab_v1*()
{
    return d->grab;
}

}
}
