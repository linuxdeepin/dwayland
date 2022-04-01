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
#include "clientmanagement_interface.h"
#include "global_p.h"
#include "display.h"
#include "logging.h"
#include "buffer_interface.h"

#include <wayland-server.h>
#include "wayland-client-management-server-protocol.h"

#define MAX_WINDOWS 100

namespace KWayland
{
namespace Server
{

class ClientManagementInterface::Private : public Global::Private
{
public:
    struct ResourceData {
        wl_resource *resource;
        uint32_t version;
    };

    Private(ClientManagementInterface *q, Display *d);

    void updateWindowStates();
    void sendWindowStates(wl_resource *resource);
    void sendWindowCaption(int windowId, bool succeed, wl_resource *buffer);

    WindowState m_windowStates[MAX_WINDOWS];
    uint32_t m_windowCount;

    QList<ResourceData> resources;
    static ClientManagementInterface *get(wl_resource *native);

private:
    void bind(wl_client *client, uint32_t version, uint32_t id) override;
    void getWindowStates();
    void captureWindowImage(int windowId, wl_resource *buffer);
    static void unbind(wl_resource *resource);
    static Private *cast(wl_resource *r) {
        return reinterpret_cast<Private*>(wl_resource_get_user_data(r));
    }

    static void getWindowStatesCallback(wl_client *client, wl_resource *resource);
    static void captureWindowImageCallback(wl_client *client, wl_resource *resource, int window_id, wl_resource *buffer);

    static const quint32 s_version;
    ClientManagementInterface *q;
    static const struct com_deepin_client_management_interface s_interface;
};

const quint32 ClientManagementInterface::Private::s_version = 1;

ClientManagementInterface::Private::Private(ClientManagementInterface *q, Display *d)
    : Global::Private(d, &com_deepin_client_management_interface, s_version)
    , q(q)
{
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct com_deepin_client_management_interface ClientManagementInterface::Private::s_interface = {
    getWindowStatesCallback,
    captureWindowImageCallback
};
#endif

ClientManagementInterface *ClientManagementInterface::Private::get(wl_resource *native)
{
    if (Private *p = cast(native)) {
        return p->q;
    }
    return nullptr;
}

ClientManagementInterface::Private *ClientManagementInterface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

ClientManagementInterface::ClientManagementInterface(Display *display, QObject *parent)
    : Global(new Private(this, display), parent)
{
    Q_D();
    connect(this, &ClientManagementInterface::windowStatesChanged, this, [this, d] { d->updateWindowStates(); });
}

ClientManagementInterface::~ClientManagementInterface() = default;

ClientManagementInterface *ClientManagementInterface::get(wl_resource* native)
{
    return Private::get(native);
}

void ClientManagementInterface::setWindowStates(QList<WindowState*> &windowStates)
{
    Q_D();
    int i = 0;
    for (auto it = windowStates.begin();
        it != windowStates.end() && i < MAX_WINDOWS;
        ++it) {
        memcpy(&d->m_windowStates[i++], *it, sizeof(WindowState));
        d->m_windowCount = i;
    }
    Q_EMIT windowStatesChanged();
}

void ClientManagementInterface::sendWindowCaptionImage(int windowId, wl_resource *buffer, QImage image)
{
    Q_D();
    bool succeed = false;
    wl_shm_buffer *shm_buffer = wl_shm_buffer_get(buffer);
    if (shm_buffer && !image.isNull()) {
        wl_shm_buffer_begin_access(shm_buffer);
        void *data = wl_shm_buffer_get_data(shm_buffer);
        if (data) {
            succeed = true;
            memcpy(data, image.bits(), image.sizeInBytes());
        }
        wl_shm_buffer_end_access(shm_buffer);
    }
    d->sendWindowCaption(windowId, succeed, buffer);
}

void ClientManagementInterface::sendWindowCaption(int windowId, wl_resource *buffer, SurfaceInterface* surface) {
    Q_D();
    if (!surface || !surface->buffer()) {
        d->sendWindowCaption(windowId, false, buffer);
        return;
    }
    bool succeed = false;
    wl_shm_buffer *shm_buffer = wl_shm_buffer_get(buffer);
    if (shm_buffer) {
        QImage image = surface->buffer()->data();
        void *data = wl_shm_buffer_get_data(shm_buffer);
        if (!image.isNull())
        {
            memcpy(data, image.bits(), image.sizeInBytes());
            succeed = true;
        }
    }
    d->sendWindowCaption(windowId, succeed, buffer);
}

void ClientManagementInterface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    auto c = display->getConnection(client);
    wl_resource *resource = c->createResource(&com_deepin_client_management_interface, qMin(version, s_version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &s_interface, this, unbind);

    ResourceData r;
    r.resource = resource;
    r.version = version;
    resources << r;
}

void ClientManagementInterface::Private::unbind(wl_resource *resource)
{
    Private *o = cast(resource);
    if (!o) {
        return;
    }
    auto it = std::find_if(o->resources.begin(), o->resources.end(), [resource](const ResourceData &r) { return r.resource == resource; });
    if (it != o->resources.end()) {
        o->resources.erase(it);
    }
}

void ClientManagementInterface::Private::getWindowStatesCallback(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    Private *p = cast(resource);
    p->getWindowStates();
}

void ClientManagementInterface::Private::captureWindowImageCallback(wl_client *client, wl_resource *resource, int windowId, wl_resource *buffer)
{
    Q_UNUSED(client)
    Private *p = cast(resource);
    p->captureWindowImage(windowId, buffer);
}

void ClientManagementInterface::Private::getWindowStates()
{
    Q_EMIT q->windowStatesRequest();
}

void ClientManagementInterface::Private::captureWindowImage(int windowId, wl_resource *buffer)
{
    qWarning() << __func__ << ":" << __LINE__ << "ut-gfx-capture: windowId " << windowId;
    Q_EMIT q->captureWindowImageRequest(windowId, buffer);
}

void ClientManagementInterface::Private::sendWindowStates(wl_resource *resource)
{
    struct wl_array data;
    auto fillArray = [this](const WindowState *origin, wl_array *dest) {
        wl_array_init(dest);
        const size_t memLength = sizeof(struct WindowState) * m_windowCount;
        void *s = wl_array_add(dest, memLength);
        memcpy(s, origin, memLength);
    };
    fillArray(m_windowStates, &data);
    com_deepin_client_management_send_window_states(resource, m_windowCount, &data);
    wl_array_release(&data);
}

void ClientManagementInterface::Private::updateWindowStates()
{
    for (auto it = resources.constBegin(); it != resources.constEnd(); ++it) {
        sendWindowStates((*it).resource);
    }
}

void ClientManagementInterface::Private::sendWindowCaption(int windowId, bool succeed, wl_resource *buffer)
{
    for (auto it = resources.constBegin(); it != resources.constEnd(); ++it) {
        qWarning() << __func__ << ":" << __LINE__ << "ut-gfx-capture-sendWindowCaption: windowId " << windowId << " resource" << (*it).resource;
        com_deepin_client_management_send_capture_callback((*it).resource, windowId, succeed, buffer);
    }
}

}
}
