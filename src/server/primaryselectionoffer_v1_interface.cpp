/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "primaryselectionoffer_v1_interface.h"
#include "primaryselectiondevice_v1_interface.h"
#include "primaryselectionsource_v1_interface.h"
#include "resource_p.h"
// Qt
#include <QStringList>
#include <QDebug>
#include <QPointer>
// Wayland
#include "wayland-wp-primary-selection-unstable-v1-server-protocol.h"
// system
#include <unistd.h>

namespace KWayland
{
namespace Server
{


class PrimarySelectionOfferV1Interface::Private : public Resource::Private
{
public:
    Private(PrimarySelectionSourceV1Interface *source, PrimarySelectionDeviceV1Interface *parentInterface, PrimarySelectionOfferV1Interface *q, wl_resource *parentResource);
    PrimarySelectionOfferV1Interface *q;
    PrimarySelectionSourceV1Interface *source;
    PrimarySelectionDeviceV1Interface *primaryDevice;


    void sendSourceActions();
protected:
private:
    PrimarySelectionOfferV1Interface *q_func() {
        return reinterpret_cast<PrimarySelectionOfferV1Interface *>(q);
    }
    void receive(const QString &mimeType, qint32 fd);
    static void receiveCallback(wl_client *client, wl_resource *resource, const char *mimeType, int32_t fd);

    static const struct zwp_primary_selection_offer_v1_interface s_interface;

};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zwp_primary_selection_offer_v1_interface PrimarySelectionOfferV1Interface::Private::s_interface = {
    receiveCallback,
    resourceDestroyedCallback,
};
#endif

PrimarySelectionOfferV1Interface::Private::Private(PrimarySelectionSourceV1Interface *source, PrimarySelectionDeviceV1Interface *parentInterface, PrimarySelectionOfferV1Interface *q, wl_resource *parentResource)
    : Resource::Private(q, nullptr, parentResource, &zwp_primary_selection_offer_v1_interface, &s_interface)
    , q(q)
    , source(source)
    , primaryDevice(parentInterface)
{
}

void PrimarySelectionOfferV1Interface::Private::receiveCallback(wl_client *client, wl_resource *resource, const char *mimeType, int32_t fd)
{
    Q_UNUSED(client)
    cast<Private>(resource)->receive(QString::fromUtf8(mimeType), fd);
}

void PrimarySelectionOfferV1Interface::Private::receive(const QString &mimeType, qint32 fd)
{
    if (!source) {
        close(fd);
        return;
    }
    source->requestData(mimeType, fd);
}

PrimarySelectionOfferV1Interface::~PrimarySelectionOfferV1Interface() = default;

void PrimarySelectionOfferV1Interface::sendAllOffers()
{
    Q_D();
    for (const QString &mimeType : d->source->mimeTypes()) {
        zwp_primary_selection_offer_v1_send_offer(d->resource,mimeType.toUtf8().constData());
    }
}

PrimarySelectionOfferV1Interface::PrimarySelectionOfferV1Interface(PrimarySelectionSourceV1Interface *source, PrimarySelectionDeviceV1Interface *parentInterface, wl_resource *parentResource)
    : Resource(new Private(source, parentInterface, this, parentResource))
{
    Q_ASSERT(source);
    connect(source, &PrimarySelectionSourceV1Interface::mimeTypeOffered, this,
        [this](const QString &mimeType) {
            Q_D();
            if (!d->resource) {
                return;
            }
            zwp_primary_selection_offer_v1_send_offer(d->resource, mimeType.toUtf8().constData());
        }
    );
    QObject::connect(source, &QObject::destroyed, this,
        [this] {
            Q_D();
            d->source = nullptr;
        }
    );
}

PrimarySelectionOfferV1Interface::Private *PrimarySelectionOfferV1Interface::d_func() const
{
        return reinterpret_cast<PrimarySelectionOfferV1Interface::Private*>(d.data());
}

}
}
