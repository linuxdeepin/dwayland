/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "primaryselectionsource_v1_interface.h"
#include "primaryselectiondevicemanager_v1_interface.h"

#include "clientconnection.h"
#include "resource_p.h"
// Qt
#include <QStringList>
// Wayland
#include "wayland-wp-primary-selection-unstable-v1-server-protocol.h"
// system
#include <unistd.h>
#include <QDebug>

namespace KWayland
{
namespace Server
{

class PrimarySelectionSourceV1Interface::Private : public Resource::Private
{
public:
    Private(PrimarySelectionSourceV1Interface *q, PrimarySelectionDeviceManagerV1Interface *parent, wl_resource *parentResource);
    virtual     ~Private();

    QStringList mimeTypes;
    PrimarySelectionSourceV1Interface* q;
protected:
    PrimarySelectionSourceV1Interface *q_func() {
        return reinterpret_cast<PrimarySelectionSourceV1Interface *>(q);
    }
    void offer(const QString &mimeType);

    const static struct zwp_primary_selection_source_v1_interface s_interface;
    static void offerCallback(wl_client *client, wl_resource *resource, const char *mimeType);
};
#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zwp_primary_selection_source_v1_interface PrimarySelectionSourceV1Interface::Private::s_interface = {
    offerCallback,
    resourceDestroyedCallback,
};
#endif

PrimarySelectionSourceV1Interface::Private::~Private() = default;

PrimarySelectionSourceV1Interface::Private::Private(PrimarySelectionSourceV1Interface *q, PrimarySelectionDeviceManagerV1Interface *parent, wl_resource *parentResource)
    : Resource::Private(q, parent, parentResource, &zwp_primary_selection_source_v1_interface, &s_interface)
    ,q(q)
{
}
void PrimarySelectionSourceV1Interface::Private::offer(const QString &mimeType)
{
    mimeTypes << mimeType;
    Q_Q(PrimarySelectionSourceV1Interface);
    emit q->mimeTypeOffered(mimeType);
}
void PrimarySelectionSourceV1Interface::Private::offerCallback(wl_client *client, wl_resource *resource, const char *mimeType)
{
    Q_UNUSED(client)
    cast<Private>(resource)->offer(QString::fromUtf8(mimeType));
}


PrimarySelectionSourceV1Interface::PrimarySelectionSourceV1Interface(PrimarySelectionDeviceManagerV1Interface *parent, ::wl_resource *parentResource)
    : AbstractDataSource(new Private(this, parent, parentResource))
{
}

PrimarySelectionSourceV1Interface::Private *PrimarySelectionSourceV1Interface::d_func() const
{
        return reinterpret_cast<PrimarySelectionSourceV1Interface::Private*>(d.data());
}

PrimarySelectionSourceV1Interface::~PrimarySelectionSourceV1Interface() = default;

void PrimarySelectionSourceV1Interface::requestData(const QString &mimeType, qint32 fd)
{
    Q_D();
    if (d->resource)
        zwp_primary_selection_source_v1_send_send(d->resource,mimeType.toUtf8().constData(),fd);
    close(fd);

}

void PrimarySelectionSourceV1Interface::cancel()
{
    Q_D();
    if (d->resource)
    {
       zwp_primary_selection_source_v1_send_cancelled(d->resource);
       client()->flush();
    }

}

QStringList PrimarySelectionSourceV1Interface::mimeTypes() const
{
    Q_D();
    return d->mimeTypes;
}

PrimarySelectionSourceV1Interface *PrimarySelectionSourceV1Interface::get(wl_resource *native)
{
    return Private::get<PrimarySelectionSourceV1Interface>(native);
}

}
}
