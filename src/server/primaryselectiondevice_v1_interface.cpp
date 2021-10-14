/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "primaryselectiondevice_v1_interface.h"
#include "primaryselectiondevicemanager_v1_interface.h"
#include "primaryselectionoffer_v1_interface.h"
#include "primaryselectionsource_v1_interface.h"
#include "wayland-wp-primary-selection-unstable-v1-server-protocol.h"
#include "display.h"
#include "seat_interface.h"
#include "seat_interface_p.h"
#include "resource_p.h"
#include <QThread>
// Wayland

namespace KWayland
{
namespace Server
{
class PrimarySelectionDeviceV1Interface::Private : public Resource::Private
{
public:
    Private(SeatInterface *seat, PrimarySelectionDeviceV1Interface *q, PrimarySelectionDeviceManagerV1Interface *manager, wl_resource *parentResource);

    PrimarySelectionOfferV1Interface *createDataOffer(PrimarySelectionSourceV1Interface *source);

    PrimarySelectionDeviceV1Interface *q;
    SeatInterface *seat;
    QPointer<PrimarySelectionSourceV1Interface> selection;
    QMetaObject::Connection selectionUnboundConnection;
    QMetaObject::Connection selectionDestroyedConnection;
private:

    PrimarySelectionDeviceV1Interface *q_func() {
        return reinterpret_cast<PrimarySelectionDeviceV1Interface*>(q);
    }
    void setSelection(PrimarySelectionSourceV1Interface *dataSource);
    static void setSelectionCallback(wl_client *client, wl_resource *resource, wl_resource *source, uint32_t serial);

    static const struct zwp_primary_selection_device_v1_interface s_interface;
protected:

};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zwp_primary_selection_device_v1_interface PrimarySelectionDeviceV1Interface::Private::s_interface = {
    setSelectionCallback,
    resourceDestroyedCallback
};
#endif

PrimarySelectionDeviceV1Interface::Private::Private(SeatInterface *seat, PrimarySelectionDeviceV1Interface *q, PrimarySelectionDeviceManagerV1Interface *manager, wl_resource *parentResource)
    : Resource::Private(q, manager, parentResource, &zwp_primary_selection_device_v1_interface, &s_interface)
    , q(q)
    , seat(seat)
{
}


void PrimarySelectionDeviceV1Interface::Private::setSelectionCallback(wl_client *client, wl_resource *resource, wl_resource *source, uint32_t serial)
{
    Q_UNUSED(client)
    Q_UNUSED(serial)
    // TODO: verify serial
    cast<Private>(resource)->setSelection(PrimarySelectionSourceV1Interface::get(source));
}

void PrimarySelectionDeviceV1Interface::Private::setSelection(PrimarySelectionSourceV1Interface *dataSource)
{
    if (selection == dataSource) {
        return;
    }
    Q_Q(PrimarySelectionDeviceV1Interface);
    QObject::disconnect(selectionUnboundConnection);
    QObject::disconnect(selectionDestroyedConnection);
    if (selection) {
        selection->cancel();
    }
    selection = dataSource;
    if (selection) {
        auto clearSelection = [this] {
            setSelection(nullptr);
        };
        selectionUnboundConnection = QObject::connect(selection, &Resource::unbound, q, clearSelection);
        selectionDestroyedConnection = QObject::connect(selection, &QObject::destroyed, q, clearSelection);
        emit q->selectionChanged(selection);
    } else {
        selectionUnboundConnection = QMetaObject::Connection();
        selectionDestroyedConnection = QMetaObject::Connection();
        emit q->selectionCleared();
    }
}

PrimarySelectionOfferV1Interface *PrimarySelectionDeviceV1Interface::Private::createDataOffer(PrimarySelectionSourceV1Interface *source)
{
    if (!resource) {
        return nullptr;
    }
    if (!source) {
        // a select offer can only exist together with a source
        return nullptr;
    }
    Q_Q(PrimarySelectionDeviceV1Interface);
    PrimarySelectionOfferV1Interface *offer = new PrimarySelectionOfferV1Interface(source, q, resource);
    auto c = q->global()->display()->getConnection(wl_resource_get_client(resource));
    offer->create(c, wl_resource_get_version(resource), 0);
    if (!offer->resource()) {
        // TODO: send error?
        delete offer;
        return nullptr;
    }
    zwp_primary_selection_device_v1_send_data_offer(resource, offer->resource());
    offer->sendAllOffers();
    return offer;
}

PrimarySelectionDeviceV1Interface::PrimarySelectionDeviceV1Interface(SeatInterface *seat, PrimarySelectionDeviceManagerV1Interface *parent, wl_resource *parentResource)
    : Resource(new Private(seat, this, parent, parentResource))
{
}

PrimarySelectionDeviceV1Interface::Private *PrimarySelectionDeviceV1Interface::d_func() const
{
        return reinterpret_cast<PrimarySelectionDeviceV1Interface::Private*>(d.data());
}

PrimarySelectionDeviceV1Interface::~PrimarySelectionDeviceV1Interface() = default;

SeatInterface *PrimarySelectionDeviceV1Interface::seat() const
{
    Q_D();
    return d->seat;
}

PrimarySelectionSourceV1Interface *PrimarySelectionDeviceV1Interface::selection() const
{
    Q_D();
    return d->selection;
}

void PrimarySelectionDeviceV1Interface::sendSelection(PrimarySelectionDeviceV1Interface *other)
{
    Q_D();
    if (!other)
        return;
    auto otherSelection = other->selection();
    if (!otherSelection) {
        sendClearSelection();
        return;
    }
    PrimarySelectionOfferV1Interface *offer = d->createDataOffer(otherSelection);
    if (!offer) {
        return;
    }
    if (!d->resource) {
        return;
    }
    zwp_primary_selection_device_v1_send_selection(d->resource,offer->resource());
}

void PrimarySelectionDeviceV1Interface::sendClearSelection()
{
    Q_D();
    if (!d->resource) {
        return;
    }
    zwp_primary_selection_device_v1_send_selection(d->resource,nullptr);
}

void PrimarySelectionDeviceV1Interface::cancel()
{
    Q_D();
    if (!d->resource) {
        return;
    }
    if (d->selection)
        d->selection->cancel();
    client()->flush();
}



}
}
