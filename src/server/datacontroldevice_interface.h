/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef WAYLAND_SERVER_DATA_CONTROL_DEVICE_INTERFACE_H
#define WAYLAND_SERVER_DATA_CONTROL_DEVICE_INTERFACE_H

#include <QObject>

#include <KWayland/Server/kwaylandserver_export.h>

#include "resource.h"
#include "datacontrolsource_interface.h"

namespace KWayland
{
namespace Server
{

class DataControlDeviceManagerInterface;
class DataControlOfferV1Interface;
class DataControlSourceV1Interface;
class SeatInterface;
class SurfaceInterface;
class PrimarySelectionSourceV1Interface;
class DataSourceInterface;
/**
 * @brief Represents the Resource for the wl_data_device interface.
 *
 * @see SeatInterface
 * @see DataSourceInterface
 **/
class KWAYLANDSERVER_EXPORT DataControlDeviceV1Interface : public Resource
{
    Q_OBJECT
public:
    virtual ~DataControlDeviceV1Interface();

    SeatInterface *seat() const;

    DataControlSourceV1Interface *selection() const;
    DataControlSourceV1Interface *primarySelection() const;

    void sendSelection(AbstractDataSource *other);
    void sendClearSelection();

    void sendPrimarySelection(AbstractDataSource *other);
    void sendClearPrimarySelection();

Q_SIGNALS:
    void selectionChanged(KWayland::Server::DataControlSourceV1Interface *dataSource);
    void selectionCleared();
    void primarySelectionChanged(KWayland::Server::DataControlSourceV1Interface *dataSource);

private:
    friend class DataControlDeviceManagerInterface;
    explicit DataControlDeviceV1Interface(SeatInterface *seat, DataControlDeviceManagerInterface *parent, wl_resource *parentResource);

    class Private;
    Private *d_func() const;
};

}
}

Q_DECLARE_METATYPE(KWayland::Server::DataControlDeviceV1Interface*)

#endif
