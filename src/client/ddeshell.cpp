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
#include "ddeshell.h"
#include "event_queue.h"
#include "logging.h"
#include "surface.h"
#include "wayland_pointer_p.h"
// Qt
#include <QDebug>
#include <QVector>
// wayland
#include "wayland-dde-shell-client-protocol.h"
#include <wayland-client-protocol.h>

namespace KWayland
{

namespace Client
{

class Q_DECL_HIDDEN DDEShell::Private
{
public:
    Private(DDEShell *q);
    void setup(dde_shell *o);

    WaylandPointer<dde_shell, dde_shell_destroy> ddeShell;
    EventQueue *queue = nullptr;

    static QVector<DDEShellSurface*> s_ddeShellSurfaces;

private:
    DDEShell *q;
};

class Q_DECL_HIDDEN DDEShellSurface::Private
{
public:
    Private(DDEShellSurface *q);
    ~Private();
    void setup(dde_shell_surface *ddeShellSurface);

    WaylandPointer<dde_shell_surface, dde_shell_surface_destroy> ddeShellSurface;
    QPointer<Surface> parentSurface;
    QRect geometry;
    bool active = false;
    bool minimized = false;
    bool maximized = false;
    bool fullscreen = false;
    bool keepAbove = false;
    bool keepBelow = false;
    bool closeable = false;
    bool minimizeable = false;
    bool maximizeable = false;
    bool fullscreenable = false;
    bool movable = false;
    bool resizable = false;

    static DDEShellSurface *get(wl_surface *surface);
    static DDEShellSurface *get(Surface *surface);

private:
    static void geometryCallback(void *data, dde_shell_surface *dde_shell_surface, int32_t x, int32_t y, uint32_t width, uint32_t height);
    static void stateChangedCallback(void *data, dde_shell_surface *dde_shell_surface, uint32_t state);
    void setActive(bool set);
    void setMinimized(bool set);
    void setMaximized(bool set);
    void setFullscreen(bool set);
    void setKeepAbove(bool set);
    void setKeepBelow(bool set);
    void setCloseable(bool set);
    void setMinimizeable(bool set);
    void setMaximizeable(bool set);
    void setFullscreenable(bool set);
    void setMovable(bool set);
    void setResizable(bool set);

    static Private *cast(void *data) {
        return reinterpret_cast<Private*>(data);
    }

    DDEShellSurface *q;
    static QVector<Private*> s_ddeShellSurfaces;
    static const dde_shell_surface_listener s_listener;
};

QVector<DDEShellSurface*> DDEShell::Private::s_ddeShellSurfaces;
QVector<DDEShellSurface::Private*> DDEShellSurface::Private::s_ddeShellSurfaces;

DDEShell::Private::Private(DDEShell *q)
    : q(q)
{
}

void DDEShell::Private::setup(dde_shell *o)
{
    Q_ASSERT(o);
    Q_ASSERT(!ddeShell);
    ddeShell.setup(o);
}

DDEShell::DDEShell(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

DDEShell::~DDEShell()
{
    d->s_ddeShellSurfaces.clear();
    d->ddeShell.release();
}

void DDEShell::setup(dde_shell *DDEShell)
{
    d->setup(DDEShell);
}

EventQueue *DDEShell::eventQueue() const
{
    return d->queue;
}

void DDEShell::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

dde_shell *DDEShell::ddeShell()
{
    return d->ddeShell;
}

bool DDEShell::isValid() const
{
    return d->ddeShell.isValid();
}

DDEShell::operator dde_shell*() {
    return d->ddeShell;
}

DDEShell::operator dde_shell*() const {
    return d->ddeShell;
}

void DDEShell::destroy()
{
    d->ddeShell.destroy();

}

DDEShellSurface *DDEShell::createShellSurface(wl_surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    auto kwS = Surface::get(surface);
    if (kwS) {
        if (auto s = DDEShellSurface::Private::get(kwS)) {
            return s;
        }
    }
    DDEShellSurface *s = new DDEShellSurface(parent);
    connect(this, &DDEShell::interfaceAboutToBeReleased, s, &DDEShellSurface::release);
    connect(this, &DDEShell::interfaceAboutToBeDestroyed, s, &DDEShellSurface::destroy);
    auto w = dde_shell_get_shell_surface(d->ddeShell, surface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    s->d->parentSurface = QPointer<Surface>(kwS);
    d->s_ddeShellSurfaces << s;
    return s;
}

DDEShellSurface *DDEShell::createShellSurface(Surface *surface, QObject *parent)
{
    return createShellSurface(*surface, parent);
}

DDEShellSurface *DDEShell::get(wl_surface *surface)
{
    if (!surface) {
        return nullptr;
    }
    for (auto it = d->s_ddeShellSurfaces.constBegin(); it != d->s_ddeShellSurfaces.constEnd(); ++it) {
        if ((*it)->d->parentSurface == Surface::get(surface)) {
            return (*it);
        }
    }
    return nullptr;
}

DDEShellSurface *DDEShell::get(Surface *surface)
{
    if (!surface) {
        return nullptr;
    }
    for (auto it = d->s_ddeShellSurfaces.constBegin(); it != d->s_ddeShellSurfaces.constEnd(); ++it) {
        if ((*it)->d->parentSurface == surface) {
            return (*it);
        }
    }
    return nullptr;
}


DDEShellSurface::Private::Private(DDEShellSurface *q)
    : q(q)
{
    s_ddeShellSurfaces << this;
}

DDEShellSurface::Private::~Private()
{
    s_ddeShellSurfaces.removeAll(this);
}

DDEShellSurface *DDEShellSurface::Private::get(wl_surface *surface)
{
    if (!surface) {
        return nullptr;
    }
    for (auto it = s_ddeShellSurfaces.constBegin(); it != s_ddeShellSurfaces.constEnd(); ++it) {
        if ((*it)->parentSurface == Surface::get(surface)) {
            return (*it)->q;
        }
    }
    return nullptr;
}

DDEShellSurface *DDEShellSurface::Private::get(Surface *surface)
{
    if (!surface) {
        return nullptr;
    }
    for (auto it = s_ddeShellSurfaces.constBegin(); it != s_ddeShellSurfaces.constEnd(); ++it) {
        if ((*it)->parentSurface == surface) {
            return (*it)->q;
        }
    }
    return nullptr;
}

void DDEShellSurface::Private::setup(dde_shell_surface *s)
{
    Q_ASSERT(s);
    Q_ASSERT(!ddeShellSurface);
    ddeShellSurface.setup(s);
    dde_shell_surface_add_listener(ddeShellSurface, &s_listener, this);
}

void DDEShellSurface::Private::stateChangedCallback(void *data, dde_shell_surface *dde_shell_surface, uint32_t state)
{
    auto p = cast(data);
    Q_UNUSED(dde_shell_surface);
    p->setActive(state & DDE_SHELL_STATE_ACTIVE);
    p->setMinimized(state & DDE_SHELL_STATE_MINIMIZED);
    p->setMaximized(state & DDE_SHELL_STATE_MAXIMIZED);
    p->setFullscreen(state & DDE_SHELL_STATE_FULLSCREEN);
    p->setKeepAbove(state & DDE_SHELL_STATE_KEEP_ABOVE);
    p->setKeepBelow(state & DDE_SHELL_STATE_KEEP_BELOW);
    p->setCloseable(state & DDE_SHELL_STATE_CLOSEABLE);
    p->setFullscreenable(state & DDE_SHELL_STATE_FULLSCREENABLE);
    p->setMaximizeable(state & DDE_SHELL_STATE_MAXIMIZABLE);
    p->setMinimizeable(state & DDE_SHELL_STATE_MINIMIZABLE);
    p->setMovable(state & DDE_SHELL_STATE_MOVABLE);
    p->setResizable(state & DDE_SHELL_STATE_RESIZABLE);
}

void DDEShellSurface::Private::geometryCallback(void *data, dde_shell_surface *ddeShellSurface, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    Q_UNUSED(ddeShellSurface)
    Private *p = cast(data);
    QRect geo(x, y, width, height);
    if (geo == p->geometry) {
        return;
    }
    p->geometry = geo;
    emit p->q->geometryChanged(geo);
}

const dde_shell_surface_listener DDEShellSurface::Private::s_listener = {
    geometryCallback,
    stateChangedCallback,
};

void DDEShellSurface::Private::setActive(bool set)
{
    if (active == set) {
        return;
    }
    active = set;
    emit q->activeChanged();
}

void DDEShellSurface::Private::setFullscreen(bool set)
{
    if (fullscreen == set) {
        return;
    }
    fullscreen = set;
    emit q->fullscreenChanged();
}

void DDEShellSurface::Private::setKeepAbove(bool set)
{
    if (keepAbove == set) {
        return;
    }
    keepAbove = set;
    emit q->keepAboveChanged();
}

void DDEShellSurface::Private::setKeepBelow(bool set)
{
    if (keepBelow == set) {
        return;
    }
    keepBelow = set;
    emit q->keepBelowChanged();
}

void DDEShellSurface::Private::setMaximized(bool set)
{
    if (maximized == set) {
        return;
    }
    maximized = set;
    emit q->maximizedChanged();
}

void DDEShellSurface::Private::setMinimized(bool set)
{
    if (minimized == set) {
        return;
    }
    minimized = set;
    emit q->minimizedChanged();
}

void DDEShellSurface::Private::setCloseable(bool set)
{
    if (closeable == set) {
        return;
    }
    closeable = set;
    emit q->closeableChanged();
}

void DDEShellSurface::Private::setFullscreenable(bool set)
{
    if (fullscreenable == set) {
        return;
    }
    fullscreenable = set;
    emit q->fullscreenableChanged();
}

void DDEShellSurface::Private::setMaximizeable(bool set)
{
    if (maximizeable == set) {
        return;
    }
    maximizeable = set;
    emit q->maximizeableChanged();
}

void DDEShellSurface::Private::setMinimizeable(bool set)
{
    if (minimizeable == set) {
        return;
    }
    minimizeable = set;
    emit q->minimizeableChanged();
}

void DDEShellSurface::Private::setMovable(bool set)
{
    if (movable == set) {
        return;
    }
    movable = set;
    emit q->movableChanged();
}

void DDEShellSurface::Private::setResizable(bool set)
{
    if (resizable == set) {
        return;
    }
    resizable = set;
    emit q->resizableChanged();
}

DDEShellSurface::DDEShellSurface(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

DDEShellSurface::~DDEShellSurface()
{
    release();
}

void DDEShellSurface::release()
{
    d->ddeShellSurface.release();
}

void DDEShellSurface::destroy()
{
    d->ddeShellSurface.destroy();
}

void DDEShellSurface::setup(dde_shell_surface *ddeShellSurface)
{
    d->setup(ddeShellSurface);
}

DDEShellSurface *DDEShellSurface::get(Surface *surface)
{
    if (auto s = DDEShellSurface::Private::get(surface)) {
        return s;
    }

    return nullptr;
}

bool DDEShellSurface::isValid() const
{
    return d->ddeShellSurface.isValid();
}

DDEShellSurface::operator dde_shell_surface*()
{
    return d->ddeShellSurface;
}

DDEShellSurface::operator dde_shell_surface*() const
{
    return d->ddeShellSurface;
}

void DDEShellSurface::requestGeometry() const
{
    dde_shell_surface_get_geometry(d->ddeShellSurface);
}

void DDEShellSurface::requestActive() const
{
    dde_shell_surface_request_active(d->ddeShellSurface);
}

void DDEShellSurface::requestActivate()
{
    dde_shell_surface_set_state(d->ddeShellSurface,
        DDE_SHELL_STATE_ACTIVE,
        DDE_SHELL_STATE_ACTIVE);
}

void DDEShellSurface::requestKeepAbove(bool set)
{
    if (!set) {
        dde_shell_surface_set_state(d->ddeShellSurface,
            DDE_SHELL_STATE_KEEP_ABOVE,
            0);
    } else {
        dde_shell_surface_set_state(d->ddeShellSurface,
            DDE_SHELL_STATE_KEEP_ABOVE,
            DDE_SHELL_STATE_KEEP_ABOVE);
    }
}

void DDEShellSurface::requestKeepBelow(bool set)
{
    if (!set) {
        dde_shell_surface_set_state(d->ddeShellSurface,
            DDE_SHELL_STATE_KEEP_BELOW,
            0);
    } else {
        dde_shell_surface_set_state(d->ddeShellSurface,
            DDE_SHELL_STATE_KEEP_BELOW,
            DDE_SHELL_STATE_KEEP_BELOW);
    }
}

void DDEShellSurface::requestMinimized(bool set)
{
    if (!set) {
        dde_shell_surface_set_state(d->ddeShellSurface,
            DDE_SHELL_STATE_MINIMIZED,
            0);
    } else {
        dde_shell_surface_set_state(d->ddeShellSurface,
            DDE_SHELL_STATE_MINIMIZED,
            DDE_SHELL_STATE_MINIMIZED);
    }
}

void DDEShellSurface::requestMaximized(bool set)
{
    if (!set) {
        dde_shell_surface_set_state(d->ddeShellSurface,
            DDE_SHELL_STATE_MAXIMIZED,
            0);
    } else {
        dde_shell_surface_set_state(d->ddeShellSurface,
            DDE_SHELL_STATE_MAXIMIZED,
            DDE_SHELL_STATE_MAXIMIZED);
    }
}

bool DDEShellSurface::isActive() const
{
    return d->active;
}

bool DDEShellSurface::isFullscreen() const
{
    return d->fullscreen;
}

bool DDEShellSurface::isKeepAbove() const
{
    return d->keepAbove;
}

bool DDEShellSurface::isKeepBelow() const
{
    return d->keepBelow;
}

bool DDEShellSurface::isMaximized() const
{
    return d->maximized;
}

bool DDEShellSurface::isMinimized() const
{
    return d->minimized;
}

bool DDEShellSurface::isCloseable() const
{
    return d->closeable;
}

bool DDEShellSurface::isFullscreenable() const
{
    return d->fullscreenable;
}

bool DDEShellSurface::isMaximizeable() const
{
    return d->maximizeable;
}

bool DDEShellSurface::isMinimizeable() const
{
    return d->minimizeable;
}

bool DDEShellSurface::isResizable() const
{
    return d->resizable;
}

bool DDEShellSurface::isMovable() const
{
    return d->movable;
}

QRect DDEShellSurface::getGeometry() const
{
    return d->geometry;
}


}
}