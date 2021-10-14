/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "primaryselectiondevicemanager_v1_interface.h"
#include "primaryselectiondevice_v1_interface.h"
#include "primaryselectionsource_v1_interface.h"
#include "wayland-wp-primary-selection-unstable-v1-server-protocol.h"
#include "display.h"
#include "seat_interface_p.h"
// Wayland
#include "global_p.h"

static const int s_version = 1;
namespace KWayland
{
namespace Server
{

class PrimarySelectionDeviceManagerV1Interface::Private : public Global::Private
{
public:
    Private(PrimarySelectionDeviceManagerV1Interface *q, Display *d);

    PrimarySelectionDeviceManagerV1Interface *q;
protected:
    void bind(wl_client *client, uint32_t version, uint32_t id) override;
    void createDataSource(wl_client *client, wl_resource *resource, uint32_t id);
    void getDataDevice(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *seat);

    static void unbind(wl_resource *resource);
    static void createDataSourceCallback(wl_client *client, wl_resource *resource, uint32_t id);
    static void getDataDeviceCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *seat);
    static Private *cast(wl_resource *r) {
        return reinterpret_cast<Private*>(wl_resource_get_user_data(r));
    }

    static const struct zwp_primary_selection_device_manager_v1_interface s_interface;
    static const quint32 s_version;
    static const qint32 s_dataDeviceVersion;
    static const qint32 s_dataSourceVersion;
};
const quint32 PrimarySelectionDeviceManagerV1Interface::Private::s_version = 1;
const qint32 PrimarySelectionDeviceManagerV1Interface::Private::s_dataDeviceVersion = 1;
const qint32 PrimarySelectionDeviceManagerV1Interface::Private::s_dataSourceVersion = 1;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zwp_primary_selection_device_manager_v1_interface PrimarySelectionDeviceManagerV1Interface::Private::s_interface = {
    createDataSourceCallback,
    getDataDeviceCallback
};
#endif


PrimarySelectionDeviceManagerV1Interface::Private::Private(PrimarySelectionDeviceManagerV1Interface *q, Display *d)
    : Global::Private(d, &zwp_primary_selection_device_manager_v1_interface, s_version)
    , q(q)
{
}


void PrimarySelectionDeviceManagerV1Interface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    auto c = display->getConnection(client);
    wl_resource *resource = c->createResource(&zwp_primary_selection_device_manager_v1_interface, qMin(version, s_version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &s_interface, this, unbind);
    // TODO: should we track?
}

void PrimarySelectionDeviceManagerV1Interface::Private::unbind(wl_resource *resource)
{
    Q_UNUSED(resource)
}

void PrimarySelectionDeviceManagerV1Interface::Private::createDataSourceCallback(wl_client *client, wl_resource *resource, uint32_t id)
{
    cast(resource)->createDataSource(client, resource, id);
}

void PrimarySelectionDeviceManagerV1Interface::Private::createDataSource(wl_client *client, wl_resource *resource, uint32_t id)
{
    PrimarySelectionSourceV1Interface *dataSource = new PrimarySelectionSourceV1Interface(q, resource);
    dataSource->create(display->getConnection(client), qMin(wl_resource_get_version(resource), s_dataSourceVersion) , id);
    if (!dataSource->resource()) {
        wl_resource_post_no_memory(resource);
        delete dataSource;
        return;
    }
    emit q->primarySelectSourceCreated(dataSource);
}

void PrimarySelectionDeviceManagerV1Interface::Private::getDataDeviceCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *seat)
{
    cast(resource)->getDataDevice(client, resource, id, seat);
}

void PrimarySelectionDeviceManagerV1Interface::Private::getDataDevice(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *seat)
{
    if (!client || !resource || !seat)
        return;
    SeatInterface *s = SeatInterface::get(seat);
    Q_ASSERT(s);
    PrimarySelectionDeviceV1Interface *dataDevice = new PrimarySelectionDeviceV1Interface(s, q, resource);
    dataDevice->create(display->getConnection(client), qMin(wl_resource_get_version(resource), s_dataDeviceVersion), id);
    if (!dataDevice->resource()) {
        wl_resource_post_no_memory(resource);
        return;
    }
    s->d_func()->registerPrimarySelectionDevice(dataDevice);
    emit q->primarySelectDeviceCreated(dataDevice);
}

PrimarySelectionDeviceManagerV1Interface::PrimarySelectionDeviceManagerV1Interface(Display *display, QObject *parent)
    : Global(new Private(this, display), parent)
{
}

PrimarySelectionDeviceManagerV1Interface::~PrimarySelectionDeviceManagerV1Interface() = default;
}
}
