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
#include "primaryselectionoffer_v1.h"
#include "primaryselectiondevice_v1.h"
#include "wayland_pointer_p.h"

// Qt
#include <QMimeType>
#include <QMimeDatabase>
// Wayland
#include "wayland-wp-primary-selection-unstable-v1-client-protocol.h"

namespace KWayland
{

namespace Client
{

class Q_DECL_HIDDEN PrimarySelectionOfferV1::Private
{
public:
    Private(zwp_primary_selection_offer_v1 *offer, PrimarySelectionOfferV1 *q);
    WaylandPointer<zwp_primary_selection_offer_v1, zwp_primary_selection_offer_v1_destroy> primarySelectionOffer;
    QList<QMimeType> mimeTypes;

private:
    void offer(const QString &mimeType);
    static void offerCallback(void *data, zwp_primary_selection_offer_v1 *primarySelectOffer, const char *mimeType);
    PrimarySelectionOfferV1 *q;

    static const struct zwp_primary_selection_offer_v1_listener s_listener;
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zwp_primary_selection_offer_v1_listener PrimarySelectionOfferV1::Private::s_listener = {
    offerCallback,
};
#endif

PrimarySelectionOfferV1::Private::Private(zwp_primary_selection_offer_v1 *offer, PrimarySelectionOfferV1 *q)
    : q(q)
{
    primarySelectionOffer.setup(offer);
    zwp_primary_selection_offer_v1_add_listener(offer, &s_listener, this);
}

void PrimarySelectionOfferV1::Private::offerCallback(void *data, zwp_primary_selection_offer_v1 *primarySelectOffer, const char *mimeType)
{
    auto d = reinterpret_cast<Private*>(data);
    Q_ASSERT(d->primarySelectionOffer == primarySelectOffer);
    d->offer(QString::fromUtf8(mimeType));
}

void PrimarySelectionOfferV1::Private::offer(const QString &mimeType)
{
    QMimeDatabase db;
    const auto &m = db.mimeTypeForName(mimeType);
    if (m.isValid()) {
        mimeTypes << m;
        emit q->mimeTypeOffered(m.name());
    }
}


PrimarySelectionOfferV1::PrimarySelectionOfferV1(PrimarySelectionDeviceV1 *parent, zwp_primary_selection_offer_v1 *dataOffer)
    : QObject(parent)
    , d(new Private(dataOffer, this))
{
}

PrimarySelectionOfferV1::~PrimarySelectionOfferV1()
{
    release();
}

void PrimarySelectionOfferV1::release()
{
    d->primarySelectionOffer.release();
}

void PrimarySelectionOfferV1::destroy()
{
    d->primarySelectionOffer.destroy();
}

bool PrimarySelectionOfferV1::isValid() const
{
    return d->primarySelectionOffer.isValid();
}

QList< QMimeType > PrimarySelectionOfferV1::offeredMimeTypes() const
{
    return d->mimeTypes;
}

void PrimarySelectionOfferV1::receive(const QMimeType &mimeType, qint32 fd)
{
    receive(mimeType.name(), fd);
}

void PrimarySelectionOfferV1::receive(const QString &mimeType, qint32 fd)
{
    Q_ASSERT(isValid());
    zwp_primary_selection_offer_v1_receive(d->primarySelectionOffer, mimeType.toUtf8().constData(), fd);
}

PrimarySelectionOfferV1::operator zwp_primary_selection_offer_v1*()
{
    return d->primarySelectionOffer;
}

PrimarySelectionOfferV1::operator zwp_primary_selection_offer_v1*() const
{
    return d->primarySelectionOffer;
}

}
}
