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
#include "ddeshell_interface.h"
#include "global_p.h"
#include "resource_p.h"
#include "display.h"
#include "logging.h"
#include "surface_interface.h"

#include <wayland-server.h>
#include "wayland-dde-shell-server-protocol.h"

#include <QDebug>
#include <QTime>

#define MAX_WINDOWS 50

namespace KWayland
{
namespace Server
{

class DDEShellInterface::Private : public Global::Private
{
public:

    Private(DDEShellInterface *q, Display *d);

    QVector<DDEShellSurfaceInterface*> ddeShellSurfaces;
    static DDEShellInterface *get(wl_resource *native);

private:
    void bind(wl_client *client, uint32_t version, uint32_t id) override;
    static Private *cast(wl_resource *r) {
        return reinterpret_cast<Private*>(wl_resource_get_user_data(r));
    }

    void getShellSurface(wl_client *client, uint32_t version, uint32_t id, SurfaceInterface *surface, wl_resource *parentSurface);
    // interface
    static void getShellSurfaceCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *surface);
    static const struct dde_shell_interface s_interface;

    static const quint32 s_version;
    DDEShellInterface *q;
};

class DDEShellSurfaceInterface::Private : public Resource::Private
{
public:
    Private(DDEShellSurfaceInterface *q, DDEShellInterface *ddeShell, SurfaceInterface *surface, wl_resource *parentResource);

    DDEShellInterface *ddeShell;
    SurfaceInterface *surface;
    QMetaObject::Connection destroyConnection;

    void setState(dde_shell_state flag, bool set);

private:
    DDEShellSurfaceInterface *q_func() {
        return reinterpret_cast<DDEShellSurfaceInterface *>(q);
    }

    static Private *cast(wl_resource *r) {
        return reinterpret_cast<Private*>(wl_resource_get_user_data(r));
    }

    quint32 m_state = 0;

    void requestGeometry(wl_resource *resource);
    void requestActive(wl_resource *resource);
    // interface
    static void requestGeometryCallback(wl_client *client, wl_resource *resource);
    static void requestActiveCallback(wl_client *client, wl_resource *resource);
    static void setStateCallback(wl_client *client, wl_resource *resource, uint32_t flags, uint32_t state);
    static void setPropertyCallback(wl_client *client, wl_resource *resource, uint32_t property, struct wl_array * dataArr);

    static const struct dde_shell_surface_interface s_interface;
};

const quint32 DDEShellInterface::Private::s_version = 1;

DDEShellInterface::Private::Private(DDEShellInterface *q, Display *d)
    : Global::Private(d, &dde_shell_interface, s_version)
    , q(q)
{
}

DDEShellInterface *DDEShellInterface::Private::get(wl_resource *native)
{
    if (Private *p = cast(native)) {
        return p->q;
    }
    return nullptr;
}

DDEShellInterface::Private *DDEShellInterface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

DDEShellInterface::DDEShellInterface(Display *display, QObject *parent)
    : Global(new Private(this, display), parent)
{
}

DDEShellInterface::~DDEShellInterface() = default;

DDEShellInterface *DDEShellInterface::get(wl_resource* native)
{
    return Private::get(native);
}

void DDEShellInterface::Private::getShellSurfaceCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *surface)
{
    auto s = reinterpret_cast<DDEShellInterface::Private*>(wl_resource_get_user_data(resource));
    s->getShellSurface(client, wl_resource_get_version(resource), id, SurfaceInterface::get(surface), resource);
}

void DDEShellInterface::Private::getShellSurface(wl_client *client, uint32_t version, uint32_t id, SurfaceInterface *surface, wl_resource *parentResource)
{
    auto it = std::find_if(ddeShellSurfaces.constBegin(), ddeShellSurfaces.constEnd(),
        [surface](DDEShellSurfaceInterface *s) {
            return surface == s->surface();
        }
    );
    if (it != ddeShellSurfaces.constEnd()) {
        wl_resource_post_error(surface->resource(), WL_DISPLAY_ERROR_INVALID_OBJECT, "DDEShellSurface already created");
        return;
    }
    DDEShellSurfaceInterface *shellSurface = new DDEShellSurfaceInterface(q, surface, parentResource);
    ddeShellSurfaces << shellSurface;
    QObject::connect(shellSurface, &DDEShellSurfaceInterface::destroyed, q,
        [this, shellSurface] {
            ddeShellSurfaces.removeAll(shellSurface);
        }
    );
    shellSurface->d->create(display->getConnection(client), version, id);
    Q_EMIT q->shellSurfaceCreated(shellSurface);
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct dde_shell_interface DDEShellInterface::Private::s_interface = {
    getShellSurfaceCallback,
};
#endif

void DDEShellInterface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    auto c = display->getConnection(client);
    wl_resource *resource = c->createResource(&dde_shell_interface, qMin(version, s_version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &s_interface, this, nullptr);
}


/*********************************
 * ShellSurfaceInterface
 *********************************/
DDEShellSurfaceInterface::Private::Private(DDEShellSurfaceInterface *q, DDEShellInterface *shell, SurfaceInterface *surface, wl_resource *parentResource)
    : Resource::Private(q, shell, parentResource, &dde_shell_surface_interface, &s_interface)
    , ddeShell(shell)
    , surface(surface)
{
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct dde_shell_surface_interface DDEShellSurfaceInterface::Private::s_interface = {
    requestGeometryCallback,
    requestActiveCallback,
    setStateCallback,
    setPropertyCallback,
};
#endif

DDEShellSurfaceInterface::DDEShellSurfaceInterface(DDEShellInterface *shell, SurfaceInterface *parent, wl_resource *parentResource)
    : Resource(new Private(this, shell, parent, parentResource))
{
    auto unsetSurface = [this] {
        Q_D();
        d->surface = nullptr;
    };
    connect(parent, &Resource::unbound, this, unsetSurface);
    connect(parent, &QObject::destroyed, this, unsetSurface);
}

DDEShellSurfaceInterface::~DDEShellSurfaceInterface() = default;

SurfaceInterface *DDEShellSurfaceInterface::surface() const {
    Q_D();
    return d->surface;
}

DDEShellInterface *DDEShellSurfaceInterface::ddeShell() const {
    Q_D();
    return d->ddeShell;
}

DDEShellSurfaceInterface::Private *DDEShellSurfaceInterface::d_func() const
{
    return reinterpret_cast<DDEShellSurfaceInterface::Private*>(d.data());
}

DDEShellSurfaceInterface *DDEShellSurfaceInterface::get(wl_resource *native)
{
    return Private::get<DDEShellSurfaceInterface>(native);
}

void DDEShellSurfaceInterface::Private::setState(dde_shell_state flag, bool set)
{
    if (!resource) {
        return;
    }
    quint32 newState = m_state;
    if (set) {
        newState |= flag;
    } else {
        newState &= ~flag;
    }
    if (newState == m_state) {
        return;
    }
    m_state = newState;
    dde_shell_surface_send_state_changed(resource, m_state);
}

void DDEShellSurfaceInterface::Private::requestGeometry(wl_resource *resource)
{
    // todo
    return;
}

void DDEShellSurfaceInterface::Private::requestGeometryCallback(wl_client *client, wl_resource *resource)
{
    auto s = cast(resource);
    Q_ASSERT(client == *(s->client));

    s->requestGeometry(resource);
}

void DDEShellSurfaceInterface::Private::requestActive(wl_resource *resource)
{
    Q_EMIT q_func()->activationRequested();
}

void DDEShellSurfaceInterface::Private::requestActiveCallback(wl_client *client, wl_resource *resource)
{
    auto s = cast(resource);
    Q_ASSERT(client == *(s->client));

    s->requestActive(resource);
}

void DDEShellSurfaceInterface::Private::setStateCallback(wl_client *client, wl_resource *resource, uint32_t flags, uint32_t state)
{
    Q_UNUSED(client)
    Private *p = cast(resource);

    if (flags & DDE_SHELL_STATE_ACTIVE) {
        Q_EMIT p->q_func()->activeRequested(state & DDE_SHELL_STATE_ACTIVE);
    }
    if (flags & DDE_SHELL_STATE_MINIMIZED) {
        Q_EMIT p->q_func()->minimizedRequested(state & DDE_SHELL_STATE_MINIMIZED);
    }
    if (flags & DDE_SHELL_STATE_MAXIMIZED) {
        Q_EMIT p->q_func()->maximizedRequested(state & DDE_SHELL_STATE_MAXIMIZED);
    }
    if (flags & DDE_SHELL_STATE_FULLSCREEN) {
        Q_EMIT p->q_func()->fullscreenRequested(state & DDE_SHELL_STATE_FULLSCREEN);
    }
    if (flags & DDE_SHELL_STATE_KEEP_ABOVE) {
        Q_EMIT p->q_func()->keepAboveRequested(state & DDE_SHELL_STATE_KEEP_ABOVE);
    }
    if (flags & DDE_SHELL_STATE_KEEP_BELOW) {
        Q_EMIT p->q_func()->keepBelowRequested(state & DDE_SHELL_STATE_KEEP_BELOW);
    }
    if (flags & DDE_SHELL_STATE_CLOSEABLE) {
        Q_EMIT p->q_func()->closeableRequested(state & DDE_SHELL_STATE_CLOSEABLE);
    }
    if (flags & DDE_SHELL_STATE_MINIMIZABLE) {
        Q_EMIT p->q_func()->minimizeableRequested(state & DDE_SHELL_STATE_MINIMIZABLE);
    }
    if (flags & DDE_SHELL_STATE_MAXIMIZABLE) {
        Q_EMIT p->q_func()->maximizeableRequested(state & DDE_SHELL_STATE_MAXIMIZABLE);
    }
    if (flags & DDE_SHELL_STATE_FULLSCREENABLE) {
        Q_EMIT p->q_func()->fullscreenableRequested(state & DDE_SHELL_STATE_FULLSCREENABLE);
    }
    if (flags & DDE_SHELL_STATE_MOVABLE) {
        Q_EMIT p->q_func()->movableRequested(state & DDE_SHELL_STATE_MOVABLE);
    }
    if (flags & DDE_SHELL_STATE_RESIZABLE) {
        Q_EMIT p->q_func()->resizableRequested(state & DDE_SHELL_STATE_RESIZABLE);
    }
    if (flags & DDE_SHELL_STATE_ACCEPT_FOCUS) {
        Q_EMIT p->q_func()->acceptFocusRequested(state & DDE_SHELL_STATE_ACCEPT_FOCUS);
    }
    if (flags & DDE_SHELL_STATE_MODALITY) {
        Q_EMIT p->q_func()->modalityRequested(state & DDE_SHELL_STATE_MODALITY);
    }
}

void DDEShellSurfaceInterface::Private::setPropertyCallback(wl_client *client, wl_resource *resource, uint32_t property, struct wl_array * dataArr)
{
    Q_UNUSED(client)
    Private *p = cast(resource);
    if (property & DDE_SHELL_PROPERTY_NOTITLEBAR) {
        int *value = static_cast<int *>(dataArr->data);
        Q_EMIT p->q_func()->noTitleBarPropertyRequested(*value);
    }
    if (property & DDE_SHELL_PROPERTY_WINDOWRADIUS) {
        float *value = static_cast<float *>(dataArr->data);
        QPointF pnt = QPointF(value[0],value[1]);
        Q_EMIT p->q_func()->windowRadiusPropertyRequested(pnt);
    }
}

void DDEShellSurfaceInterface::setActive(bool set)
{
    Q_D();
    d->setState(DDE_SHELL_STATE_ACTIVE, set);
}

void DDEShellSurfaceInterface::setFullscreen(bool set)
{
    Q_D();
    d->setState(DDE_SHELL_STATE_FULLSCREEN, set);
}

void DDEShellSurfaceInterface::setKeepAbove(bool set)
{
    Q_D();
    d->setState(DDE_SHELL_STATE_KEEP_ABOVE, set);
}

void DDEShellSurfaceInterface::setKeepBelow(bool set)
{
    Q_D();
    d->setState(DDE_SHELL_STATE_KEEP_BELOW, set);
}

void DDEShellSurfaceInterface::setMaximized(bool set)
{
    Q_D();
    d->setState(DDE_SHELL_STATE_MAXIMIZED, set);
}

void DDEShellSurfaceInterface::setMinimized(bool set)
{
    Q_D();
    d->setState(DDE_SHELL_STATE_MINIMIZED, set);
}

void DDEShellSurfaceInterface::setCloseable(bool set)
{
    Q_D();
    d->setState(DDE_SHELL_STATE_CLOSEABLE, set);
}

void DDEShellSurfaceInterface::setFullscreenable(bool set)
{
    Q_D();
    d->setState(DDE_SHELL_STATE_FULLSCREENABLE, set);
}

void DDEShellSurfaceInterface::setMaximizeable(bool set)
{
    Q_D();
    d->setState(DDE_SHELL_STATE_MAXIMIZABLE, set);
}

void DDEShellSurfaceInterface::setMinimizeable(bool set)
{
    Q_D();
    d->setState(DDE_SHELL_STATE_MINIMIZABLE, set);
}

void DDEShellSurfaceInterface::setMovable(bool set)
{
    Q_D();
    d->setState(DDE_SHELL_STATE_MOVABLE, set);
}

void DDEShellSurfaceInterface::setResizable(bool set)
{
    Q_D();
    d->setState(DDE_SHELL_STATE_RESIZABLE, set);
}

void DDEShellSurfaceInterface::setAcceptFocus(bool set)
{
    Q_D();
    d->setState(DDE_SHELL_STATE_ACCEPT_FOCUS, set);
}

void DDEShellSurfaceInterface::setModal(bool set)
{
    Q_D();
    d->setState(DDE_SHELL_STATE_MODALITY, set);
}

void DDEShellSurfaceInterface::sendGeometry(const QRect &geom)
{
    Q_D();
    if (!d->resource) {
        return;
    }
    if (geometry == geom) {
        return;
    }
    geometry = geom;
    if (!geometry.isValid()) {
        return;
    }
    dde_shell_surface_send_geometry(d->resource, geometry.x(), geometry.y(), geometry.width(), geometry.height());
}

}
}
