/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "datacontroldevicemanager_interface.h"
#include "datacontroldevice_interface.h"
#include "datacontrolsource_interface.h"

#include "global_p.h"
#include "display.h"
#include "seat_interface_p.h"
// Wayland
#include <wayland-wlr-data-control-unstable-v1-server-protocol.h>

namespace KWayland
{
namespace Server
{

class DataControlDeviceManagerInterface::Private : public Global::Private
{
public:
    Private(DataControlDeviceManagerInterface *q, Display *d);

private:
    void bind(wl_client *client, uint32_t version, uint32_t id) override;
    void createDataSource(wl_client *client, wl_resource *resource, uint32_t id);
    void getDataDevice(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *seat);

    static void unbind(wl_resource *resource);
    static void createDataSourceCallback(wl_client *client, wl_resource *resource, uint32_t id);
    static void getDataDeviceCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *seat);
    static Private *cast(wl_resource *r) {
        return reinterpret_cast<Private*>(wl_resource_get_user_data(r));
    }

    DataControlDeviceManagerInterface *q;
    static const struct zwlr_data_control_manager_v1_interface s_interface;
    static const quint32 s_version;
    static const qint32 s_dataDeviceVersion;
    static const qint32 s_dataSourceVersion;
};

const quint32 DataControlDeviceManagerInterface::Private::s_version = 1;
const qint32 DataControlDeviceManagerInterface::Private::s_dataDeviceVersion = 1;
const qint32 DataControlDeviceManagerInterface::Private::s_dataSourceVersion = 1;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zwlr_data_control_manager_v1_interface DataControlDeviceManagerInterface::Private::s_interface = {
    createDataSourceCallback,
    getDataDeviceCallback,
};
#endif

DataControlDeviceManagerInterface::Private::Private(DataControlDeviceManagerInterface *q, Display *d)
    : Global::Private(d, &zwlr_data_control_manager_v1_interface, s_version)
    , q(q)
{
}

void DataControlDeviceManagerInterface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    auto c = display->getConnection(client);
    wl_resource *resource = c->createResource(&zwlr_data_control_manager_v1_interface, qMin(version, s_version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &s_interface, this, unbind);
}

void DataControlDeviceManagerInterface::Private::unbind(wl_resource *resource)
{
    Q_UNUSED(resource)
}

void DataControlDeviceManagerInterface::Private::createDataSourceCallback(wl_client *client, wl_resource *resource, uint32_t id)
{
    cast(resource)->createDataSource(client, resource, id);
}

void DataControlDeviceManagerInterface::Private::createDataSource(wl_client *client, wl_resource *resource, uint32_t id)
{
    DataControlSourceV1Interface *dataSource = new DataControlSourceV1Interface(q, resource);
    dataSource->create(display->getConnection(client), qMin(wl_resource_get_version(resource), s_dataSourceVersion) , id);
    if (!dataSource->resource()) {
        wl_resource_post_no_memory(resource);
        delete dataSource;
        return;
    }
    emit q->dataSourceCreated(dataSource);
}

void DataControlDeviceManagerInterface::Private::getDataDeviceCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *seat)
{
    cast(resource)->getDataDevice(client, resource, id, seat);
}

void DataControlDeviceManagerInterface::Private::getDataDevice(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *seat)
{
    SeatInterface *s = SeatInterface::get(seat);
    Q_ASSERT(s);
    DataControlDeviceV1Interface *dataDevice = new DataControlDeviceV1Interface(s, q, resource);
    dataDevice->create(display->getConnection(client), qMin(wl_resource_get_version(resource), s_dataDeviceVersion), id);
    if (!dataDevice->resource()) {
        wl_resource_post_no_memory(resource);
        return;
    }
    s->d_func()->registerDataControlDevice(dataDevice);
    emit q->dataDeviceCreated(dataDevice);
}

DataControlDeviceManagerInterface::DataControlDeviceManagerInterface(Display *display, QObject *parent)
    : Global(new Private(this, display), parent)
{
}

DataControlDeviceManagerInterface::~DataControlDeviceManagerInterface() = default;

}
}
