/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef WAYLAND_SERVER_DATA_CONTROL_SOURCE_INTERFACE_H
#define WAYLAND_SERVER_DATA_CONTROL_SOURCE_INTERFACE_H

#include <QObject>

#include <KWayland/Server/kwaylandserver_export.h>

#include "abstract_data_source.h"
#include "datacontroldevicemanager_interface.h"



namespace KWayland
{
namespace Server
{

/**
 * @brief Represents the Resource for the wl_data_source interface.
 **/
class KWAYLANDSERVER_EXPORT DataControlSourceV1Interface : public AbstractDataSource
{
    Q_OBJECT
public:
    virtual ~DataControlSourceV1Interface();

    void requestData(const QString &mimeType, qint32 fd) override;
    void cancel() override;

    QStringList mimeTypes() const override;

    static DataControlSourceV1Interface *get(wl_resource *native);

private:
    friend class DataControlDeviceManagerInterface;
    explicit DataControlSourceV1Interface(DataControlDeviceManagerInterface *parent, wl_resource *parentResource);

    class Private;
    Private *d_func() const;
};

}
}

Q_DECLARE_METATYPE(KWayland::Server::DataControlSourceV1Interface*)

#endif
