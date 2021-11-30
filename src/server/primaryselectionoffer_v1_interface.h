/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_SERVER_PRIMARY_SELECTION_OFFER_INTERFACE_H
#define WAYLAND_SERVER_PRIMARY_SELECTION_OFFER_INTERFACE_H

#include <QObject>

#include <KWayland/Server/kwaylandserver_export.h>

#include "primaryselectiondevicemanager_v1_interface.h"

#include "resource.h"

struct wl_resource;
namespace KWayland
{
namespace Server
{

class AbstractDataSource;
class PrimarySelectionDeviceV1Interface;
class PrimarySelectionSourceV1Interface;
class PrimarySelectionOfferV1InterfacePrivate;

/**
 * @brief Represents the Resource for the wl_data_offer interface.
 * Lifespan is mapped to the underlying object
 **/
class KWAYLANDSERVER_EXPORT PrimarySelectionOfferV1Interface : public Resource
{
    Q_OBJECT
public:
    virtual ~PrimarySelectionOfferV1Interface() ;

    void sendAllOffers();

private:
    friend class PrimarySelectionDeviceV1Interface;
    explicit PrimarySelectionOfferV1Interface(AbstractDataSource *source, PrimarySelectionDeviceV1Interface *parentInterface, wl_resource *parentResource);
    class Private;
    Private *d_func() const;
};

}
}
Q_DECLARE_METATYPE(KWayland::Server::PrimarySelectionOfferV1Interface*)

#endif
