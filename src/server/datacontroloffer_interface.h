/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef WAYLAND_SERVER_DATA_CONTROL_OFFER_INTERFACE_H
#define WAYLAND_SERVER_DATA_CONTROL_OFFER_INTERFACE_H

#include <QObject>

#include <KWayland/Server/kwaylandserver_export.h>

#include "resource.h"
#include "datacontroldevicemanager_interface.h"

namespace KWayland
{
namespace Server
{

class DataControlDeviceV1Interface;
class DataControlSourceV1Interface;
class AbstractDataSource;

/**
 * @brief Represents the Resource for the wl_data_offer interface.
 *
 **/
class KWAYLANDSERVER_EXPORT DataControlOfferV1Interface : public Resource
{
    Q_OBJECT
public:

    enum SourceType
    {
        SourceType_Data,
        SourceType_Primary,
        SourceType_Control
    };

    virtual ~DataControlOfferV1Interface();

    void sendAllOffers();

private:
    friend class DataControlDeviceV1Interface;
    explicit DataControlOfferV1Interface(AbstractDataSource *source, DataControlDeviceV1Interface *parentInterface, wl_resource *parentResource);

    class Private;
    Private *d_func() const;
};

}
}

Q_DECLARE_METATYPE(KWayland::Server::DataControlOfferV1Interface*)

#endif
