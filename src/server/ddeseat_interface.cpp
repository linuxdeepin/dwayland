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

private:
    void bind(wl_client *client, uint32_t version, uint32_t id) override;
    static Private *cast(wl_resource *r) {
        return reinterpret_cast<Private*>(wl_resource_get_user_data(r));
    }

    void getPointer(wl_client *client, wl_resource *resource, uint32_t id);
    // interface
    static void getPointerCallback(wl_client *client, wl_resource *resource, uint32_t id);
    static const struct dde_seat_interface s_interface;

    static const quint32 s_version;
    static const qint32 s_ddePointerVersion;
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

const quint32 DDESeatInterface::Private::s_version = 1;
const qint32 DDESeatInterface::Private::s_ddePointerVersion = 1;

DDESeatInterface::Private::Private(DDESeatInterface *q, Display *d)
    : Global::Private(d, &dde_seat_interface, s_version)
    , q(q)
{
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct dde_seat_interface DDESeatInterface::Private::s_interface = {
    getPointerCallback,
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
    emit q->ddePointerCreated(ddePointer);
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
    emit pointerPosChanged(pos);
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

}
}
