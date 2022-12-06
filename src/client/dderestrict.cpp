/********************************************************************
Copyright 2020  pengwenhao <pengwenhao@uniontech.com>

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
#include "dderestrict.h"
#include "event_queue.h"
#include "logging.h"
#include "surface.h"
#include "wayland_pointer_p.h"
// Qt
#include <QDebug>
// wayland
#include "wayland-dde-restrict-client-protocol.h"
#include <wayland-client-protocol.h>

namespace KWayland
{

namespace Client
{

class Q_DECL_HIDDEN DDERestrict::Private
{
public:
    explicit Private(DDERestrict *q);
    void setup(dde_restrict *d);

    EventQueue *m_queue = nullptr;
    WaylandPointer<dde_restrict, dde_restrict_destroy> m_ddeRestrict;

private:
    static void logedCallback(void *data, dde_restrict *dde_restrict, const char *log);
    static const struct dde_restrict_listener s_listener;

    DDERestrict *q;
};

const dde_restrict_listener DDERestrict::Private::s_listener = {
    logedCallback
};

void DDERestrict::Private::logedCallback(void *data, dde_restrict *dde_restrict, const char *log)
{
    Q_UNUSED(dde_restrict)
    emit reinterpret_cast<Private*>(data)->q->windowLoged(log);
}

DDERestrict::Private::Private(DDERestrict *q)
    : q(q)
{
}

void DDERestrict::Private::setup(dde_restrict *o)
{
    Q_ASSERT(o);
    Q_ASSERT(!m_ddeRestrict);
    m_ddeRestrict.setup(o);
    dde_restrict_add_listener(m_ddeRestrict, &s_listener, this);
}

DDERestrict::DDERestrict(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

DDERestrict::~DDERestrict()
{
    release();
}

bool DDERestrict::isValid() const
{
    if (!d->m_ddeRestrict) {
        return false;
    }
    return d->m_ddeRestrict.isValid();
}

void DDERestrict::destroy()
{
    if (!d->m_ddeRestrict) {
        return;
    }
    d->m_ddeRestrict.destroy();
}

void DDERestrict::release()
{
    if (!d->m_ddeRestrict) {
        return;
    }
    d->m_ddeRestrict.release();
}

void DDERestrict::setup(dde_restrict *manager)
{
    d->setup(manager);
}

void DDERestrict::setEventQueue(EventQueue *queue)
{
    d->m_queue = queue;
}

EventQueue *DDERestrict::eventQueue() const
{
    return d->m_queue;
}

DDERestrict::operator dde_restrict*() {
    return d->m_ddeRestrict;
}

DDERestrict::operator dde_restrict*() const {
    return d->m_ddeRestrict;
}

void DDERestrict::switchScreencast(SwitchFlags state)
{
    Q_ASSERT(isValid());
    dde_restrict_switch_screencast(d->m_ddeRestrict, state);
}

void DDERestrict::setClientWhiteList(const QString& whitelist)
{
    Q_ASSERT(isValid());
    dde_restrict_client_whitelist(d->m_ddeRestrict, whitelist.toLatin1());
}

}
}
