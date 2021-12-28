/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "datacontroloffer_interface.h"
#include "datacontrolsource_interface.h"
#include "datacontroldevicemanager_interface.h"
#include "resource_p.h"

#include "datasource_interface.h"
#include "primaryselectionsource_v1_interface.h"
#include "abstract_data_source.h"

// Qt
#include <QStringList>
// Wayland
#include <wayland-wlr-data-control-unstable-v1-server-protocol.h>
// system
#include <unistd.h>

namespace KWayland
{
namespace Server
{

class Q_DECL_HIDDEN DataControlOfferV1Interface::Private : public Resource::Private
{
public:
    Private(AbstractDataSource* datasource, DataControlDeviceV1Interface *parentInterface, DataControlOfferV1Interface *q, wl_resource *parentResource);
    ~Private();

    AbstractDataSource* source;
    DataControlDeviceV1Interface *dataControlDevice;

private:
    DataControlOfferV1Interface *q_func() {
        return reinterpret_cast<DataControlOfferV1Interface *>(q);
    }
    void receive(const QString &mimeType, qint32 fd);
    static void receiveCallback(wl_client *client, wl_resource *resource, const char *mimeType, int32_t fd);
    static const struct zwlr_data_control_offer_v1_interface s_interface;

};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zwlr_data_control_offer_v1_interface DataControlOfferV1Interface::Private::s_interface = {
    receiveCallback,
    resourceDestroyedCallback,
};
#endif

DataControlOfferV1Interface::Private::Private(AbstractDataSource* datasource, DataControlDeviceV1Interface *parentInterface, DataControlOfferV1Interface *q, wl_resource *parentResource)
    : Resource::Private(q, nullptr, parentResource, &zwlr_data_control_offer_v1_interface, &s_interface)
    , dataControlDevice(parentInterface)
    ,source(datasource)
{

}


DataControlOfferV1Interface::Private::~Private() = default;


void DataControlOfferV1Interface::Private::receiveCallback(wl_client *client, wl_resource *resource, const char *mimeType, int32_t fd)
{
    Q_UNUSED(client)
    cast<Private>(resource)->receive(QString::fromUtf8(mimeType), fd);
}

void DataControlOfferV1Interface::Private::receive(const QString &mimeType, qint32 fd)
{
    Q_UNUSED(resource)
    if (!source) {
        close(fd);
        return;
    }
    source->requestData(mimeType, fd);
}





DataControlOfferV1Interface::DataControlOfferV1Interface(AbstractDataSource *source, DataControlDeviceV1Interface *parentInterface, wl_resource *parentResource)
    : Resource(new Private(source,parentInterface, this, parentResource))
{
    Q_D();
    Q_ASSERT(source);
    connect(source, &DataSourceInterface::mimeTypeOffered,
        [this](const QString &mimeType) {
            Q_D();
            if (!d->resource) {
                return;
            }
            zwlr_data_control_offer_v1_send_offer(d->resource, mimeType.toUtf8().constData());
        }
    );
    QObject::connect(source, &QObject::destroyed, this,
        [this] {
            Q_D();
            d->source = nullptr;
        }
    );
}

DataControlOfferV1Interface::~DataControlOfferV1Interface() = default;

void DataControlOfferV1Interface::sendAllOffers()
{
    Q_D();

    for (const QString &mimeType : d->source->mimeTypes()) {
        zwlr_data_control_offer_v1_send_offer(d->resource, mimeType.toUtf8().constData());
    }
}

DataControlOfferV1Interface::Private *DataControlOfferV1Interface::d_func() const
{
    return reinterpret_cast<DataControlOfferV1Interface::Private*>(d.data());
}


}
}
