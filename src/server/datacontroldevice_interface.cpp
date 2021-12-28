/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "datacontroldevice_interface.h"
#include "datacontroldevicemanager_interface.h"
#include "datacontroloffer_interface.h"
#include "display.h"
#include "resource_p.h"
#include "pointer_interface.h"
#include "seat_interface.h"
#include "surface_interface.h"
// Wayland
#include <wayland-wlr-data-control-unstable-v1-server-protocol.h>

namespace KWayland
{
namespace Server
{

class DataControlDeviceV1Interface::Private : public Resource::Private
{
public:
    Private(SeatInterface *seat, DataControlDeviceV1Interface *q, DataControlDeviceManagerInterface *manager, wl_resource *parentResource);
    ~Private();

    DataControlOfferV1Interface *createDataOffer(AbstractDataSource *source);

    SeatInterface *seat;
    DataControlSourceV1Interface *select = nullptr;
    DataControlSourceV1Interface *primarySelection = nullptr;
    QMetaObject::Connection selectionUnboundConnection;
    QMetaObject::Connection selectionDestroyedConnection;

private:
    DataControlDeviceV1Interface *q_func() {
        return reinterpret_cast<DataControlDeviceV1Interface*>(q);
    }
    void setSelection(DataControlSourceV1Interface *dataSource);
    void setPrimarySelection(DataControlSourceV1Interface *dataSource);
    void destroy(DataControlSourceV1Interface *dataSource);

    static void setSelectionCallback(struct wl_client *client,struct wl_resource *resource,struct wl_resource *source);
    static void destroyCallback(struct wl_client *client,struct wl_resource *resource);
    static void setPrimarySelectionCallback(struct wl_client *client, struct wl_resource *resource, struct wl_resource *source);

    static const struct zwlr_data_control_device_v1_interface s_interface;
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zwlr_data_control_device_v1_interface DataControlDeviceV1Interface::Private::s_interface = {
    setSelectionCallback,
    resourceDestroyedCallback,
    setPrimarySelectionCallback
};
#endif

DataControlDeviceV1Interface::Private::Private(SeatInterface *seat, DataControlDeviceV1Interface *q, DataControlDeviceManagerInterface *manager, wl_resource *parentResource)
    : Resource::Private(q, manager, parentResource, &zwlr_data_control_device_v1_interface, &s_interface)
    , seat(seat)
{
}

DataControlDeviceV1Interface::Private::~Private() = default;



void DataControlDeviceV1Interface::Private::setSelectionCallback(struct wl_client *client,struct wl_resource *resource,struct wl_resource *source)
{
    Q_UNUSED(client)
    // TODO: verify serial
    cast<Private>(resource)->setSelection(DataControlSourceV1Interface::get(source));
}

void DataControlDeviceV1Interface::Private::setPrimarySelectionCallback(struct wl_client *client,struct wl_resource *resource,struct wl_resource *source)
{
    Q_UNUSED(client)
    // TODO: verify serial
    cast<Private>(resource)->setPrimarySelection(DataControlSourceV1Interface::get(source));
}

void DataControlDeviceV1Interface::Private::setSelection(DataControlSourceV1Interface *dataSource)
{
    if (dataSource && (dataSource == seat->selection() || dataSource == seat->primarySelection())) {
        return;
    }
    if (select == dataSource) {
        return;
    }
    Q_Q(DataControlDeviceV1Interface);
    QObject::disconnect(selectionUnboundConnection);
    QObject::disconnect(selectionDestroyedConnection);
    if (select) {
        select->cancel();
    }
    select = dataSource;
    if (select) {
        auto clearSelection = [this] {
            setSelection(nullptr);
        };
        selectionUnboundConnection = QObject::connect(select, &Resource::unbound, q, clearSelection);
        selectionDestroyedConnection = QObject::connect(select, &QObject::destroyed, q, clearSelection);
        emit q->selectionChanged(select);
    } else {
        selectionUnboundConnection = QMetaObject::Connection();
        selectionDestroyedConnection = QMetaObject::Connection();
        emit q->selectionCleared();
    }
}

void DataControlDeviceV1Interface::Private::setPrimarySelection(DataControlSourceV1Interface *dataSource)
{
    Q_ASSERT(dataSource);
    if (dataSource) {
        if (dataSource == seat->selection() || dataSource == seat->primarySelection()) {
            //thx wl_resource_post_error(resource->handle, error::error_used_source, "source given to set_primary_selection was already used before");
            return;
        }
    }
    if (primarySelection) {
        primarySelection->cancel();
    }
    primarySelection = dataSource;
    emit q_func()->primarySelectionChanged(primarySelection);
}

DataControlOfferV1Interface *DataControlDeviceV1Interface::Private::createDataOffer(AbstractDataSource *source)
{
    if (!source) {
        // a data offer can only exist together with a source
        return nullptr;
    }

    if (!resource) {
        return nullptr;
    }

    DataControlOfferV1Interface *offer = new DataControlOfferV1Interface(source ,q_func(), resource);
    auto c = q->global()->display()->getConnection(wl_resource_get_client(resource));
    offer->create(c, wl_resource_get_version(resource), 0);
    if (!offer->resource()) {
        // TODO: send error?
        delete offer;
        return nullptr;
    }
    zwlr_data_control_device_v1_send_data_offer(resource ,offer->resource());
    offer->sendAllOffers();
    return offer;

}

DataControlDeviceV1Interface::DataControlDeviceV1Interface(SeatInterface *seat, DataControlDeviceManagerInterface *parent, wl_resource *parentResource)
    : Resource(new Private(seat, this, parent, parentResource))
{
}

DataControlDeviceV1Interface::~DataControlDeviceV1Interface() = default;

SeatInterface *DataControlDeviceV1Interface::seat() const
{
    Q_D();
    return d->seat;
}

DataControlSourceV1Interface *DataControlDeviceV1Interface::selection() const
{
    Q_D();
    return d->select;
}

DataControlSourceV1Interface *DataControlDeviceV1Interface::primarySelection() const
{
    Q_D();
    return d->primarySelection;
}

void DataControlDeviceV1Interface::sendSelection(AbstractDataSource *other)
{
    Q_D();
    if (!other) {
        sendClearSelection();
        return;
    }
    DataControlOfferV1Interface *offer = d->createDataOffer(other);
    if (!offer) {
        return;
    }
    zwlr_data_control_device_v1_send_selection(d->resource, offer->resource());

}

void DataControlDeviceV1Interface::sendClearSelection()
{
     zwlr_data_control_device_v1_send_selection(d->resource,nullptr);
}

void DataControlDeviceV1Interface::sendPrimarySelection(AbstractDataSource *other)
{
    Q_D();
    if (!other) {
        sendClearSelection();
        return;
    }
    DataControlOfferV1Interface *offer = d->createDataOffer(other);
    if (!offer) {
        return;
    }
    zwlr_data_control_device_v1_send_primary_selection(d->resource, offer->resource());
}

void DataControlDeviceV1Interface::sendClearPrimarySelection()
{
    zwlr_data_control_device_v1_send_primary_selection(d->resource,nullptr);
}

DataControlDeviceV1Interface::Private *DataControlDeviceV1Interface::d_func() const
{
    return reinterpret_cast<DataControlDeviceV1Interface::Private*>(d.data());
}

}
}
