// Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
// SPDX-FileCopyrightText: 2022 Martin Gräßlin <mgraesslin@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

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
