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
#ifndef WAYLAND_PRIMARY_SELECTOPM_OFFER_V1_H
#define WAYLAND_PRIMARY_SELECTOPM_OFFER_V1_H

#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

#include "primaryselectiondevicemanager_v1.h"

struct zwp_primary_selection_offer_v1;

class QMimeType;

namespace KWayland
{
namespace Client
{
class PrimarySelectionDeviceV1;

/**
 * @short Wrapper for the wl_data_offer interface.
 *
 * This class is a convenient wrapper for the wl_data_offer interface.
 * The DataOffer gets created by DataDevice.
 *
 * @see DataOfferManager
 **/
class KWAYLANDCLIENT_EXPORT PrimarySelectionOfferV1 : public QObject
{
    Q_OBJECT
public:
    virtual ~PrimarySelectionOfferV1();

    /**
     * Releases the wl_data_offer interface.
     * After the interface has been released the DataOffer instance is no
     * longer valid and can be setup with another wl_data_offer interface.
     **/
    void release();
    /**
     * Destroys the data held by this DataOffer.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new wl_data_offer interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * DataOffer gets destroyed.
     *
     * @see release
     **/
    void destroy();
    /**
     * @returns @c true if managing a wl_data_offer.
     **/
    bool isValid() const;

    QList<QMimeType> offeredMimeTypes() const;

    void receive(const QMimeType &mimeType, qint32 fd);
    void receive(const QString &mimeType, qint32 fd);

    operator zwp_primary_selection_offer_v1*();
    operator zwp_primary_selection_offer_v1*() const;

Q_SIGNALS:
    void mimeTypeOffered(const QString&);


private:
    friend class PrimarySelectionDeviceV1;
    explicit PrimarySelectionOfferV1(PrimarySelectionDeviceV1 *parent, zwp_primary_selection_offer_v1 *dataOffer);
    class Private;
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(KWayland::Client::PrimarySelectionOfferV1*)

#endif
