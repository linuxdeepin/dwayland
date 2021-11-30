/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "datacontrolsource_interface.h"
#include "datacontroldevicemanager_interface.h"
#include "clientconnection.h"
#include "resource_p.h"
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

class DataControlSourceV1Interface::Private : public Resource::Private
{
public:
    Private(DataControlSourceV1Interface *q, DataControlDeviceManagerInterface *parent, wl_resource *parentResource);
    ~Private();

    QStringList mimeTypes;

private:
    DataControlSourceV1Interface *q_func() {
        return reinterpret_cast<DataControlSourceV1Interface *>(q);
    }
    void offer(const QString &mimeType);

    static void offerCallback(wl_client *client, wl_resource *resource, const char *mimeType);

    const static struct zwlr_data_control_source_v1_interface s_interface;
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zwlr_data_control_source_v1_interface DataControlSourceV1Interface::Private::s_interface = {
    offerCallback,
    resourceDestroyedCallback
};
#endif

DataControlSourceV1Interface::Private::Private(DataControlSourceV1Interface *q, DataControlDeviceManagerInterface *parent, wl_resource *parentResource)
    : Resource::Private(q, parent, parentResource, &zwlr_data_control_source_v1_interface, &s_interface)
{
}

DataControlSourceV1Interface::Private::~Private() = default;

void DataControlSourceV1Interface::Private::offerCallback(wl_client *client, wl_resource *resource, const char *mimeType)
{
    Q_UNUSED(client)
    cast<Private>(resource)->offer(QString::fromUtf8(mimeType));
}

void DataControlSourceV1Interface::Private::offer(const QString &mimeType)
{
    mimeTypes << mimeType;
    Q_Q(DataControlSourceV1Interface);
    emit q->mimeTypeOffered(mimeType);
}

DataControlSourceV1Interface::~DataControlSourceV1Interface() = default;


void DataControlSourceV1Interface::requestData(const QString &mimeType, qint32 fd)
{
    Q_D();
    if (d->resource) {
        zwlr_data_control_source_v1_send_send(d->resource, mimeType.toUtf8().constData(), int32_t(fd));
    }
    close(fd);
}

void DataControlSourceV1Interface::cancel()
{
    Q_D();
    if (!d->resource) {
        return;
    }
    zwlr_data_control_source_v1_send_cancelled(d->resource);
    client()->flush();
}

QStringList DataControlSourceV1Interface::mimeTypes() const
{
    Q_D();
    return d->mimeTypes;
}

DataControlSourceV1Interface *DataControlSourceV1Interface::get(wl_resource *native)
{
    return Private::get<DataControlSourceV1Interface>(native);
}

DataControlSourceV1Interface::DataControlSourceV1Interface(DataControlDeviceManagerInterface *parent, wl_resource *parentResource)
    : AbstractDataSource(new Private(this, parent, parentResource))
{

}

DataControlSourceV1Interface::Private *DataControlSourceV1Interface::d_func() const
{
    return reinterpret_cast<DataControlSourceV1Interface::Private*>(d.data());
}



}
}
