/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef WAYLAND_SERVER_DATA_CONTROL_DEVICE_MANAGER_INTERFACE_H
#define WAYLAND_SERVER_DATA_CONTROL_DEVICE_MANAGER_INTERFACE_H

#include <QObject>

#include <KWayland/Server/kwaylandserver_export.h>
#include "global.h"


namespace KWayland
{
namespace Server
{

class Display;
class DataControlSourceV1Interface;
class DataControlDeviceManagerInterfacePrivate;
class DataControlDeviceV1Interface;


/**
 * @brief Represents the Global for zwlr_data_control_manager_v1 interface.
 *
 **/
class KWAYLANDSERVER_EXPORT DataControlDeviceManagerInterface : public Global
{
    Q_OBJECT
public:
    ~DataControlDeviceManagerInterface() override;

Q_SIGNALS:
    void dataSourceCreated(KWayland::Server::DataControlSourceV1Interface *dataSource);
    void dataDeviceCreated(KWayland::Server::DataControlDeviceV1Interface *dataDevice);

private:
    explicit DataControlDeviceManagerInterface(Display *display, QObject *parent = nullptr);
    friend class Display;
    class Private;
};

}
}

#endif
