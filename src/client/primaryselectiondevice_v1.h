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
#ifndef WAYLAND_PRIMARY_SELECTION_OFFER_V1_H
#define WAYLAND_PRIMARY_SELECTION_OFFER_V1_H

#include "primaryselectionoffer_v1.h"

#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

struct zwp_primary_selection_device_v1;

namespace KWayland
{
namespace Client
{
class PrimarySelectionSourceV1;

class Surface;

/**
 * @short Wrapper for the wl_data_device interface.
 *
 * This class is a convenient wrapper for the wl_data_device interface.
 * To create a DataDevice call DataDeviceManager::getDataDevice.
 *
 * @see DataDeviceManager
 **/
class KWAYLANDCLIENT_EXPORT PrimarySelectionDeviceV1 : public QObject
{
    Q_OBJECT
public:
    explicit PrimarySelectionDeviceV1(QObject *parent = nullptr);
    virtual ~PrimarySelectionDeviceV1();

    /**
     * Setup this PrimarySelectionDeviceV1 to manage the @p dataDevice.
     * When using DataDeviceManager::createDataDevice there is no need to call this
     * method.
     **/
    void setup(zwp_primary_selection_device_v1 *primarySelectDevice);
    /**
     * Releases the wl_data_device interface.
     * After the interface has been released the DataDevice instance is no
     * longer valid and can be setup with another wl_data_device interface.
     **/
    void release();
    /**
     * Destroys the data held by this DataDevice.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new wl_data_device interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * DataDevice gets destroyed.
     *
     * @see release
     **/
    void destroy();
    /**
     * @returns @c true if managing a wl_data_device.
     **/
    bool isValid() const;


    void setSelection(quint32 serial, PrimarySelectionSourceV1 *source = nullptr);
    void clearSelection(quint32 serial);

    PrimarySelectionOfferV1 *offeredSelection() const;

    operator zwp_primary_selection_device_v1*();
    operator zwp_primary_selection_device_v1*() const;

Q_SIGNALS:
    void selectionOffered(KWayland::Client::PrimarySelectionOfferV1*);
    void selectionCleared();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
