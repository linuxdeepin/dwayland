// Copyright 2017  David Edmundson <kde@davidedmundson.co.uk>
// SPDX-FileCopyrightText: 2022 Martin Gräßlin <mgraesslin@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "xwayland_keyboard_grab_v1_interface.h"
#include "display.h"
#include "surface_interface.h"
#include "seat_interface.h"
#include "global_p.h"
#include "resource_p.h"
#include "logging.h"

#include <QtGlobal>

#include <wayland-xwayland-keyboard-grab-v1-server-protocol.h>

namespace KWayland
{
namespace Server
{
class ZWPXwaylandKeyboardGrabManagerV1Interface::Private : public Global::Private
{
public:
    Private(ZWPXwaylandKeyboardGrabManagerV1Interface *q, Display *d);

    ZWPXwaylandKeyboardGrabV1Interface* grab;
private:
    void bind(wl_client *client, uint32_t version, uint32_t id) override;

    static void unbind(wl_resource *resource);
    static Private *cast(wl_resource *r) {
        return reinterpret_cast<Private*>(wl_resource_get_user_data(r));
    }

    static void grabKeyboardCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource * surface, wl_resource * seat);
    static void destroyCallback(wl_client *client, wl_resource *resource);

    ZWPXwaylandKeyboardGrabManagerV1Interface *q;
    static const struct zwp_xwayland_keyboard_grab_manager_v1_interface s_interface;
    static const quint32 s_version;
};

const quint32 ZWPXwaylandKeyboardGrabManagerV1Interface::Private::s_version = 1;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zwp_xwayland_keyboard_grab_manager_v1_interface ZWPXwaylandKeyboardGrabManagerV1Interface::Private::s_interface = {
    destroyCallback,
    grabKeyboardCallback
};
#endif

void ZWPXwaylandKeyboardGrabManagerV1Interface::Private::destroyCallback(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

void ZWPXwaylandKeyboardGrabManagerV1Interface::Private::grabKeyboardCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource * surface, wl_resource * seat)
{
    auto p = reinterpret_cast<Private*>(wl_resource_get_user_data(resource));
    Q_ASSERT(p);

    SurfaceInterface *s = SurfaceInterface::get(surface);
    if (!s) {
        // TODO: send error?
        qCWarning(KWAYLAND_SERVER) << "ServerSideDecorationInterface requested for non existing SurfaceInterface";
        return;
    }

    SeatInterface *st = SeatInterface::get(seat);
    if (!st) {
        // TODO: send error?
        qCWarning(KWAYLAND_SERVER) << "ServerSideDecorationInterface requested for non existing SeatInterface";
        return;
    }
    auto grab = new ZWPXwaylandKeyboardGrabV1Interface(p->q, s, st, resource);
    grab->create(p->display->getConnection(client), wl_resource_get_version(resource), id);
    if (!grab->resource()) {
        wl_resource_post_no_memory(resource);
        delete grab;
        return;
    }
    p->grab = grab;
    QObject::connect(grab, &QObject::destroyed, p->q, [=]() {
        p->grab = nullptr;
        emit p->q->zwpXwaylandKeyboardGrabV1Destroyed();
    });
    emit p->q->zwpXwaylandKeyboardGrabV1Created(grab);
}

ZWPXwaylandKeyboardGrabManagerV1Interface::Private::Private(ZWPXwaylandKeyboardGrabManagerV1Interface *q, Display *d)
    : Global::Private(d, &zwp_xwayland_keyboard_grab_manager_v1_interface, s_version)
    , q(q)
{
}

void ZWPXwaylandKeyboardGrabManagerV1Interface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    auto c = display->getConnection(client);
    wl_resource *resource = c->createResource(&zwp_xwayland_keyboard_grab_manager_v1_interface, qMin(version, s_version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &s_interface, this, unbind);
}

void ZWPXwaylandKeyboardGrabManagerV1Interface::Private::unbind(wl_resource *resource)
{
    Q_UNUSED(resource)
}

class ZWPXwaylandKeyboardGrabV1Interface::Private : public Resource::Private
{
public:
    Private(ZWPXwaylandKeyboardGrabV1Interface *q, ZWPXwaylandKeyboardGrabManagerV1Interface *c, SurfaceInterface *surface, SeatInterface *seat, wl_resource *parentResource);
    ~Private();


    SurfaceInterface *surface;
    SeatInterface *seat;
private:

    ZWPXwaylandKeyboardGrabV1Interface *q_func()
    {
        return reinterpret_cast<ZWPXwaylandKeyboardGrabV1Interface *>(q);
    }
    static ZWPXwaylandKeyboardGrabV1Interface *get(SurfaceInterface *s);

    static void resourceDestroyedCallback(wl_client *client, wl_resource *resource);
    static const struct zwp_xwayland_keyboard_grab_v1_interface s_interface;
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zwp_xwayland_keyboard_grab_v1_interface ZWPXwaylandKeyboardGrabV1Interface::Private::s_interface = {
    resourceDestroyedCallback
};
#endif

void ZWPXwaylandKeyboardGrabV1Interface::Private::resourceDestroyedCallback(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

ZWPXwaylandKeyboardGrabV1Interface::Private::Private(ZWPXwaylandKeyboardGrabV1Interface *q, ZWPXwaylandKeyboardGrabManagerV1Interface *c, SurfaceInterface *s, SeatInterface *seat, wl_resource *parentResource)
    : Resource::Private(q, c, parentResource, &zwp_xwayland_keyboard_grab_v1_interface, &s_interface),
    surface(s),
    seat(seat)
{
}

ZWPXwaylandKeyboardGrabV1Interface::Private::~Private()
{
    if (resource) {
        wl_resource_destroy(resource);
        resource = nullptr;
    }
}

ZWPXwaylandKeyboardGrabManagerV1Interface::ZWPXwaylandKeyboardGrabManagerV1Interface(Display *display, QObject *parent)
    : Global(new Private(this, display), parent)
{
}

ZWPXwaylandKeyboardGrabManagerV1Interface::~ZWPXwaylandKeyboardGrabManagerV1Interface()
{
}

ZWPXwaylandKeyboardGrabV1Interface* ZWPXwaylandKeyboardGrabManagerV1Interface::getGrabClient()
{
    Q_D();
    return d->grab;
}

ZWPXwaylandKeyboardGrabManagerV1Interface::Private *ZWPXwaylandKeyboardGrabManagerV1Interface::d_func() const
{
    return reinterpret_cast<ZWPXwaylandKeyboardGrabManagerV1Interface::Private*>(d.data());
}


ZWPXwaylandKeyboardGrabV1Interface::ZWPXwaylandKeyboardGrabV1Interface(ZWPXwaylandKeyboardGrabManagerV1Interface *parent, SurfaceInterface *s, SeatInterface *seat, wl_resource *parentResource):
    Resource(new Private(this, parent, s, seat, parentResource))
{
}

ZWPXwaylandKeyboardGrabV1Interface::Private *ZWPXwaylandKeyboardGrabV1Interface::d_func() const
{
    return reinterpret_cast<ZWPXwaylandKeyboardGrabV1Interface::Private*>(d.data());
}

ZWPXwaylandKeyboardGrabV1Interface::~ZWPXwaylandKeyboardGrabV1Interface()
{}


SurfaceInterface* ZWPXwaylandKeyboardGrabV1Interface::surface() const
{
    Q_D();
    return d->surface;
}

SeatInterface* ZWPXwaylandKeyboardGrabV1Interface::seat() const
{
    Q_D();
    return d->seat;
}

}//namespace
}

