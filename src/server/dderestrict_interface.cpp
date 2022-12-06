/********************************************************************
Copyright 2015  Martin Gräßlin <mgraesslin@kde.org>

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
#include "dderestrict_interface.h"
#include "display.h"
#include "global_p.h"
#include "resource_p.h"

#include <functional>
#include <wayland-server.h>
#include "wayland-dde-restrict-server-protocol.h"

namespace KWayland
{
namespace Server
{

class DDERestrictInterface::Private : public Global::Private
{
public:
    Private(DDERestrictInterface *q, Display *d);
    static DDERestrictInterface *get(wl_resource *native);

    void setProhibitScreencast(bool set);
    void setClientWhitelists(const QList<QByteArray> &whitelists);

    bool m_prohibitScreencast = false;
    QList<QByteArray> m_clientWhitelists;

private:
    void bind(wl_client *client, uint32_t version, uint32_t id) override;
    static void switchScreencastCallback(wl_client *client, wl_resource *resource, uint32_t switch_flag);
    static void clientWhitelistCallback(wl_client *client, wl_resource *resource, const char *white_str);
    static void unbind(wl_resource *resource);
    static Private *cast(wl_resource *r) {
        return reinterpret_cast<Private*>(wl_resource_get_user_data(r));
    }

    DDERestrictInterface *q;
    static const struct dde_restrict_interface s_interface;
    static const quint32 s_version;
};

const quint32 DDERestrictInterface::Private::s_version = 1;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct dde_restrict_interface DDERestrictInterface::Private::s_interface = {
    switchScreencastCallback,
    clientWhitelistCallback
};
#endif

DDERestrictInterface::Private::Private(DDERestrictInterface *q, Display *d)
    : Global::Private(d, &dde_restrict_interface, s_version)
    , q(q)
{
}

DDERestrictInterface *DDERestrictInterface::Private::get(wl_resource *native)
{
    if (Private *p = cast(native)) {
        return p->q;
    }
    return nullptr;
}

DDERestrictInterface::Private *DDERestrictInterface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

void DDERestrictInterface::Private::clientWhitelistCallback(wl_client *client, wl_resource *resource, const char *white_str)
{
    Q_UNUSED(client);
    if (white_str == nullptr)
        return;

    QByteArray byte(white_str);
    QList<QByteArray> white_lists;

    if (byte.contains(","))
        white_lists = byte.split(',');
    else {
        if (!byte.isEmpty()) white_lists.append(byte);
    }

    cast(resource)->setClientWhitelists(white_lists);
}

void DDERestrictInterface::Private::switchScreencastCallback(wl_client *client, wl_resource *resource, uint32_t switch_flag)
{
    Q_UNUSED(client);
    cast(resource)->setProhibitScreencast(switch_flag == dde_restrict_switch_flag::DDE_RESTRICT_SWITCH_FLAG_OFF);
}

void DDERestrictInterface::Private::setProhibitScreencast(bool set)
{
    if(m_prohibitScreencast == set)
        return;

    m_prohibitScreencast = set;
}

void DDERestrictInterface::Private::setClientWhitelists(const QList<QByteArray> &whitelists)
{
    if(m_clientWhitelists == whitelists)
        return;

    m_clientWhitelists = whitelists;
}

void DDERestrictInterface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    auto c = display->getConnection(client);
    wl_resource *resource = c->createResource(&dde_restrict_interface, qMin(version, s_version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &s_interface, this, unbind);
    // TODO: should we track?
}

void DDERestrictInterface::Private::unbind(wl_resource *resource)
{
    Q_UNUSED(resource);
}

DDERestrictInterface::DDERestrictInterface(Display *display, QObject *parent)
    : Global(new Private(this, display), parent)
{
}

DDERestrictInterface::~DDERestrictInterface() = default;

DDERestrictInterface *DDERestrictInterface::get(wl_resource* native)
{
    return Private::get(native);
}

bool DDERestrictInterface::prohibitScreencast()
{
    return d_func()->m_prohibitScreencast;
}

QList<QByteArray> DDERestrictInterface::clientWhitelists()
{
    return d_func()->m_clientWhitelists;
}

}
}
