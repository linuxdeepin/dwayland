/********************************************************************
Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#ifndef WAYLAND_PRIMARY_SELECTION_SOURCE_V1_H
#define WAYLAND_PRIMARY_SELECTION_SOURCE_V1_H

#include "buffer.h"
#include "primaryselectiondevicemanager_v1.h"

#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

struct zwp_primary_selection_source_v1;
class QMimeType;

namespace KWayland
{
namespace Client
{


/**
 * @short Wrapper for the zwp_primary_selection_source_v1 interface.
 *
 * This class is a convenient wrapper for the zwp_primary_selection_source_v1 interface.
 * To create a DataSource call DataDeviceManager::createDataSource.
 *
 * @see DataDeviceManager
 **/
class KWAYLANDCLIENT_EXPORT PrimarySelectionSourceV1 : public QObject
{
    Q_OBJECT
public:
    explicit PrimarySelectionSourceV1(QObject *parent = nullptr);
    virtual ~PrimarySelectionSourceV1();

    /**
     * Setup this DataSource to manage the @p dataSource.
     * When using DataDeviceManager::createDataSource there is no need to call this
     * method.
     **/
    void setup(zwp_primary_selection_source_v1 *dataSource);
    /**
     * Releases the zwp_primary_selection_source_v1 interface.
     * After the interface has been released the DataSource instance is no
     * longer valid and can be setup with another zwp_primary_selection_source_v1 interface.
     **/
    void release();
    /**
     * Destroys the data held by this DataSource.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zwp_primary_selection_source_v1 interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * DataSource gets destroyed.
     *
     * @see release
     **/
    void destroy();
    /**
     * @returns @c true if managing a zwp_primary_selection_source_v1.
     **/
    bool isValid() const;

    void offer(const QString &mimeType);
    void offer(const QMimeType &mimeType);

    operator zwp_primary_selection_source_v1*();
    operator zwp_primary_selection_source_v1*() const;

Q_SIGNALS:

    /**
     * Request for data from the client. Send the data as the
     * specified @p mimeType over the passed file descriptor @p fd, then close
     * it.
     **/
    void sendDataRequested(const QString &mimeType, qint32 fd);
    /**
     * This DataSource has been replaced by another DataSource.
     * The client should clean up and destroy this DataSource.
     **/
    void cancelled();


private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
