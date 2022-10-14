// Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
// SPDX-FileCopyrightText: 2022 Martin Gräßlin <mgraesslin@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef WAYLAND_SERVER_DATA_SOURCE_INTERFACE_H
#define WAYLAND_SERVER_DATA_SOURCE_INTERFACE_H

#include <QObject>

#include <KWayland/Server/kwaylandserver_export.h>

#include "abstract_data_source.h"
#include "datadevicemanager_interface.h"

namespace KWayland
{
namespace Server
{

/**
 * @brief Represents the Resource for the wl_data_source interface.
 **/
class KWAYLANDSERVER_EXPORT DataSourceInterface : public AbstractDataSource
{
    Q_OBJECT
public:
    virtual ~DataSourceInterface();

    void accept(const QString &mimeType) override;
    void requestData(const QString &mimeType, qint32 fd) override;
    void cancel() override;

    QStringList mimeTypes() const override;

    static DataSourceInterface *get(wl_resource *native);

    /**
     * @returns The Drag and Drop actions supported by this DataSourceInterface.
     * @since 5.42
     **/
    DataDeviceManagerInterface::DnDActions supportedDragAndDropActions() const override;

    /**
     * The user performed the drop action during a drag and drop operation.
     * @since 5.42
     **/
    void dropPerformed() override;
    /**
     * The drop destination finished interoperating with this data source.
     * @since 5.42
     **/
    void dndFinished() override;
    /**
     * This event indicates the @p action selected by the compositor after matching the
     * source/destination side actions. Only one action (or none) will be offered here.
     * @since 5.42
     **/
    void dndAction(DataDeviceManagerInterface::DnDAction action) override;

private:
    friend class DataDeviceManagerInterface;
    explicit DataSourceInterface(DataDeviceManagerInterface *parent, wl_resource *parentResource);

    class Private;
    Private *d_func() const;
};

}
}

Q_DECLARE_METATYPE(KWayland::Server::DataSourceInterface*)

#endif
