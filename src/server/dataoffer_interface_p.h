// Copyright 2017  Martin Flöser <mgraesslin@kde.org>
// SPDX-FileCopyrightText: 2022 Martin Gräßlin <mgraesslin@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef KWAYLAND_SERVER_DATAOFFERINTERFACE_P_H
#define KWAYLAND_SERVER_DATAOFFERINTERFACE_P_H
#include "dataoffer_interface.h"
#include "datasource_interface.h"
#include "resource_p.h"
#include <wayland-server.h>

namespace KWayland
{
namespace Server
{

class Q_DECL_HIDDEN DataOfferInterface::Private : public Resource::Private
{
public:
    Private(AbstractDataSource *source, DataDeviceInterface *parentInterface, DataOfferInterface *q, wl_resource *parentResource);
    ~Private();
    AbstractDataSource *source;
    DataDeviceInterface *dataDevice;
    // defaults are set to sensible values for < version 3 interfaces
    DataDeviceManagerInterface::DnDActions supportedDnDActions = DataDeviceManagerInterface::DnDAction::Copy | DataDeviceManagerInterface::DnDAction::Move;
    DataDeviceManagerInterface::DnDAction preferredDnDAction = DataDeviceManagerInterface::DnDAction::Copy;

    void sendSourceActions();

private:
    DataOfferInterface *q_func() {
        return reinterpret_cast<DataOfferInterface *>(q);
    }
    void receive(const QString &mimeType, qint32 fd);
    static void acceptCallback(wl_client *client, wl_resource *resource, uint32_t serial, const char *mimeType);
    static void receiveCallback(wl_client *client, wl_resource *resource, const char *mimeType, int32_t fd);
    static void finishCallback(wl_client *client, wl_resource *resource);
    static void setActionsCallback(wl_client *client, wl_resource *resource, uint32_t dnd_actions, uint32_t preferred_action);

    static const struct wl_data_offer_interface s_interface;
};

}
}

#endif
