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
#include "clientmanagement.h"
#include "logging.h"
#include "wayland_pointer_p.h"
// Qt
#include <QDebug>
#include <QVector>
// wayland
#include "wayland-client-management-client-protocol.h"
#include <wayland-client-protocol.h>

namespace KWayland
{

namespace Client
{

typedef QVector<ClientManagement::WindowState> WindowStates;

class Q_DECL_HIDDEN ClientManagement::Private
{
public:
    Private(ClientManagement *q);
    void setup(com_deepin_client_management *o);
    void get_window_states();

    WaylandPointer<com_deepin_client_management, com_deepin_client_management_destroy> clientManagement;
    EventQueue *queue = nullptr;
    uint m_windowsCount;
    WindowStates m_windowStates;

    uint32_t m_windowFromPoint;
    QVector<uint32_t> m_allWindowId;
    WindowState m_specificWindowState;

private:
    static void windowStatesCallback(void *data, com_deepin_client_management *clientManagement, uint32_t count, wl_array *windowStates);
    static void windowFromPointCallback(void *data, com_deepin_client_management *clientManagement, uint32_t window_id);
    static void allWindowIdCallback(void *data, com_deepin_client_management *clientManagement, wl_array *id_array);
    static void specificWindowStateCallback(void *data, com_deepin_client_management *clientManagement, int32_t pid,
            uint32_t window_id, const char *resource_name, int32_t x, int32_t y, int32_t width, int32_t height,
            int32_t is_minimized, int32_t is_fullscreen, int32_t is_active, int32_t splitable, const char *uuid);
    void addWindowStates(uint32_t count, wl_array *windowStates);

    ClientManagement *q;
    static struct com_deepin_client_management_listener s_clientManagementListener;
};

ClientManagement::Private::Private(ClientManagement *q)
    : q(q)
{
}

void ClientManagement::Private::get_window_states()
{
    Q_ASSERT(clientManagement);
    com_deepin_client_management_get_window_states(clientManagement);
}

void ClientManagement::Private::setup(com_deepin_client_management *o)
{
    Q_ASSERT(o);
    Q_ASSERT(!clientManagement);
    clientManagement.setup(o);
    com_deepin_client_management_add_listener(clientManagement, &s_clientManagementListener, this);
}

ClientManagement::ClientManagement(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

ClientManagement::~ClientManagement()
{
    d->clientManagement.release();
}

com_deepin_client_management_listener ClientManagement::Private::s_clientManagementListener = {
    .window_states = windowStatesCallback,
    .window_from_point = windowFromPointCallback,
    .all_window_id = allWindowIdCallback,
    .specific_window_state = specificWindowStateCallback,
};

void ClientManagement::Private::addWindowStates(uint32_t count, wl_array *windowStates)
{
    m_windowsCount = count;

    if (0 < windowStates->size && (0 == (windowStates->size % sizeof(ClientManagement::WindowState)))) {
        m_windowStates.clear();
        m_windowStates.resize(m_windowsCount);
        memcpy(m_windowStates.data(), windowStates->data, windowStates->size);
        Q_EMIT q->windowStatesChanged();
    } else {
        qWarning() << Q_FUNC_INFO << "receive wayland event error";
    }
}

void ClientManagement::Private::windowStatesCallback(void *data, com_deepin_client_management *clientManagement,
                                                uint32_t count,
                                                wl_array *windowStates)
{
    Q_UNUSED(clientManagement);
    auto o = reinterpret_cast<ClientManagement::Private*>(data);
    o->addWindowStates(count, windowStates);
}

void ClientManagement::Private::windowFromPointCallback(void *data, com_deepin_client_management *clientManagement, uint32_t window_id)
{
    Q_UNUSED(clientManagement);
    auto o = reinterpret_cast<ClientManagement::Private*>(data);
    o->m_windowFromPoint = window_id;
}

void ClientManagement::Private::allWindowIdCallback(void *data, com_deepin_client_management *clientManagement, wl_array *id_array)
{
    Q_UNUSED(clientManagement);
    auto o = reinterpret_cast<ClientManagement::Private*>(data);
    if (0 < id_array->size && (0 == (id_array->size % sizeof(ClientManagement::WindowState)))) {
        o->m_allWindowId.clear();
        o->m_allWindowId.resize(id_array->size / sizeof(ClientManagement::WindowState));
        memcpy(o->m_allWindowId.data(), id_array->data, id_array->size);
    } else {
        qWarning() << Q_FUNC_INFO << "receive wayland event error";
    }
}

void ClientManagement::Private::specificWindowStateCallback(void *data, com_deepin_client_management *clientManagement, int32_t pid,
            uint32_t window_id, const char *resource_name, int32_t x, int32_t y, int32_t width, int32_t height,
            int32_t is_minimized, int32_t is_fullscreen, int32_t is_active, int32_t splitable, const char *uuid)
{
    Q_UNUSED(clientManagement);
    auto o = reinterpret_cast<ClientManagement::Private*>(data);
    o->m_specificWindowState = {
        .pid = pid,
        .windowId = static_cast<int32_t>(window_id),
        .resourceName = {0},
        .geometry = {
            .x = x,
            .y = y,
            .width = width,
            .height = height
        },
        .isMinimized = static_cast<bool>(is_minimized),
        .isFullScreen = static_cast<bool>(is_fullscreen),
        .isActive = static_cast<bool>(is_active)
    };
    int length = strlen(resource_name);
    std::copy_n(resource_name,
            std::min(static_cast<int>(sizeof(o->m_specificWindowState.resourceName) - 1), length),
            o->m_specificWindowState.resourceName);
}

void ClientManagement::setup(com_deepin_client_management *clientManagement)
{
    d->setup(clientManagement);
}

EventQueue *ClientManagement::eventQueue() const
{
    return d->queue;
}

void ClientManagement::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

com_deepin_client_management *ClientManagement::clientManagement()
{
    return d->clientManagement;
}

bool ClientManagement::isValid() const
{
    return d->clientManagement.isValid();
}

ClientManagement::operator com_deepin_client_management*() {
    return d->clientManagement;
}

ClientManagement::operator com_deepin_client_management*() const {
    return d->clientManagement;
}

void ClientManagement::destroy()
{
    d->clientManagement.destroy();

}

const QVector <ClientManagement::WindowState> &ClientManagement::getWindowStates() const
{
    if (d->m_windowStates.empty()) {
        qDebug() << "now m_windowStates is empty send get_window_states request to server";
        d->get_window_states();
    }
    return d->m_windowStates;
}

}
}
