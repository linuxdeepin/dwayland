/********************************************************************
Copyright 2015  Martin Gräßlin <mgraesslin@kde.org>
Copyright 2015  Marco Martin <mart@kde.org>

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
#include "strut_interface.h"
#include "region_interface.h"
#include "display.h"
#include "global_p.h"
#include "resource_p.h"
#include "surface_interface_p.h"
#include <wayland-server.h>
#include <wayland-strut-server-protocol.h>

namespace KWayland
{
namespace Server
{
class StrutInterface::Private : public Global::Private
{
public:
    struct ResourceData {
        wl_resource *resource;
        uint32_t version;
    };
    Private(StrutInterface *q, Display *d);
    ~Private();

private:
    void bind(wl_client *client, uint32_t version, uint32_t id) override;

    static void setStrutPartialCallback(wl_client *client, wl_resource *resource, wl_resource *surface,
                                        int32_t left,
                                        int32_t right,
                                        int32_t top,
                                        int32_t bottom,
                                        int32_t left_start_y,
                                        int32_t left_end_y,
                                        int32_t right_start_y,
                                        int32_t right_end_y,
                                        int32_t top_start_x,
                                        int32_t top_end_x,
                                        int32_t bottom_start_x,
                                        int32_t bottom_end_x);

    static void unbind(wl_resource *resource);
    static Private *cast(wl_resource *r) {
        if (!r) {
            return nullptr;
        }
        auto strut = reinterpret_cast<QPointer<StrutInterface>*>(wl_resource_get_user_data(r))->data();
        if (strut) {
            return static_cast<Private*>(strut->d.data());
        }
        return nullptr;
    }

    StrutInterface *get(wl_resource *native);
    StrutInterface *q;
    static const struct com_deepin_kwin_strut_interface s_interface;
    static const quint32 s_version;
};

const quint32 StrutInterface::Private::s_version = 1;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct com_deepin_kwin_strut_interface StrutInterface::Private::s_interface = {
    setStrutPartialCallback,
};
#endif

StrutInterface::Private::Private(StrutInterface *q, Display *d)
    : Global::Private(d, &com_deepin_kwin_strut_interface, s_version)
    , q(q)
{
}

StrutInterface::Private::~Private() = default;


void StrutInterface::Private::setStrutPartialCallback(wl_client *client,
                                                      wl_resource *resource,
                                                      wl_resource *surface,
                                                      int32_t left,
                                                      int32_t right,
                                                      int32_t top,
                                                      int32_t bottom,
                                                      int32_t left_start_y,
                                                      int32_t left_end_y,
                                                      int32_t right_start_y,
                                                      int32_t right_end_y,
                                                      int32_t top_start_x,
                                                      int32_t top_end_x,
                                                      int32_t bottom_start_x,
                                                      int32_t bottom_end_x)
{
    Q_UNUSED(client);
    if (!resource || !surface) {
        return;
    }
    struct deepinKwinStrut kwinStrut(left,
                                     right,
                                     top,
                                     bottom,
                                     left_start_y,
                                     left_end_y,
                                     right_start_y,
                                     right_end_y,
                                     top_start_x,
                                     top_end_x,
                                     bottom_start_x,
                                     bottom_end_x);
    auto m = cast(resource);
    if (!m) {
        return;
    }
    SurfaceInterface *si = SurfaceInterface::get(surface);
    if (m->q) {
        Q_EMIT m->q->setStrut(si, kwinStrut);
    }
}

StrutInterface *StrutInterface::Private::get(wl_resource *native)
{
    if (!native) {
        return nullptr;
    }

    if (Private *p = cast(native)) {
        return p->q;
    }
    return nullptr;
}

void StrutInterface::Private::unbind(wl_resource *r)
{
    if (!r) {
        return;
    }
    delete reinterpret_cast<QPointer<StrutInterface>*>(wl_resource_get_user_data(r));
}

void StrutInterface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    if (!client) {
        return;
    }
    auto c = display->getConnection(client);
    if (!c) {
        return;
    }
    wl_resource *resource = c->createResource(&com_deepin_kwin_strut_interface, qMin(version, s_version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    auto ref = new QPointer<StrutInterface>(q);//deleted in unbind
    if (!ref) {
        return;
    }
    wl_resource_set_implementation(resource, &s_interface, ref, unbind);
}


StrutInterface::StrutInterface(Display *display, QObject *parent)
    : Global(new Private(this, display), parent)
{
}

StrutInterface::~StrutInterface() = default;

}
}
