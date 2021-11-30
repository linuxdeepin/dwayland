/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_SERVER_PRIMARY_SELECTION_DEVICE_INTERFACE_H
#define WAYLAND_SERVER_PRIMARY_SELECTION_DEVICE_INTERFACE_H

#include <QObject>

#include <KWayland/Server/kwaylandserver_export.h>
#include "resource.h"

struct wl_resource;
struct wl_client;

namespace KWayland
{
namespace Server
{

class PrimarySelectionDeviceManagerV1Interface;
class PrimarySelectionOfferV1Interface;
class PrimarySelectionSourceV1Interface;
class SeatInterface;
class SurfaceInterface;
class PrimarySelectionDeviceV1InterfacePrivate;
class AbstractDataSource;

/**
 * @brief Represents the Resource for the wl_data_device interface.
 *
 * @see SeatInterface
 * @see PrimarySelectionSourceInterface
 * Lifespan is mapped to the underlying object
 **/
class KWAYLANDSERVER_EXPORT PrimarySelectionDeviceV1Interface : public Resource
{
    Q_OBJECT
public:
    virtual ~PrimarySelectionDeviceV1Interface();

    SeatInterface *seat() const;

    PrimarySelectionSourceV1Interface *selection() const;

    void sendSelection(AbstractDataSource *other);
    void sendClearSelection();
    void cancel();

Q_SIGNALS:
    void selectionChanged(KWayland::Server::PrimarySelectionSourceV1Interface*);
    void selectionCleared();

private:
    friend class PrimarySelectionDeviceManagerV1Interface;
    explicit PrimarySelectionDeviceV1Interface(SeatInterface *seat, PrimarySelectionDeviceManagerV1Interface *parent,  wl_resource *resource);


    class Private;
    Private *d_func() const;
};

}
}
Q_DECLARE_METATYPE(KWayland::Server::PrimarySelectionDeviceV1Interface*)

#endif
