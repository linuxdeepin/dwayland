/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_SERVER_PRIMARY_SELECTION_DEVICE_MANAGER_INTERFACE_H
#define WAYLAND_SERVER_PRIMARY_SELECTION_DEVICE_MANAGER_INTERFACE_H

#include <QObject>
#include "global.h"
#include <KWayland/Server/kwaylandserver_export.h>
namespace KWayland
{
namespace Server
{

class Display;
class PrimarySelectionSourceV1Interface;
class PrimarySelectionDeviceManagerV1InterfacePrivate;
class PrimarySelectionDeviceV1Interface;

/**
 * @brief Represents the Global for zwp_primary_selection_manager_v1 interface.
 *
 **/
class KWAYLANDSERVER_EXPORT PrimarySelectionDeviceManagerV1Interface : public Global
{
    Q_OBJECT
public:
    ~PrimarySelectionDeviceManagerV1Interface();

Q_SIGNALS:
    void primarySelectSourceCreated(KWayland::Server::PrimarySelectionSourceV1Interface *dataSource);
    void primarySelectDeviceCreated(KWayland::Server::PrimarySelectionDeviceV1Interface *dataDevice);

private:
    explicit PrimarySelectionDeviceManagerV1Interface(Display *display, QObject *parent = nullptr);
    friend class Display;
    class Private;
};

}
}
#endif
