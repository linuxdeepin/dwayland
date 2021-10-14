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
#include "primaryselectionsource_v1.h"
#include "wayland_pointer_p.h"
// Qt
#include <QMimeType>
// Wayland
#include "wayland-wp-primary-selection-unstable-v1-client-protocol.h"



namespace KWayland
{
namespace Client
{

class Q_DECL_HIDDEN PrimarySelectionSourceV1::Private
{
public:
    explicit Private(PrimarySelectionSourceV1 *q);
    void setup(zwp_primary_selection_source_v1 *s);

    WaylandPointer<zwp_primary_selection_source_v1, zwp_primary_selection_source_v1_destroy> source;


private:

    static void sendCallback(void *data, zwp_primary_selection_source_v1 *primarySelectSource, const char *mimeType, int32_t fd);
    static void cancelledCallback(void *data, zwp_primary_selection_source_v1 *primarySelectSource);

    static const struct zwp_primary_selection_source_v1_listener s_listener;

    PrimarySelectionSourceV1 *q;
};

const zwp_primary_selection_source_v1_listener PrimarySelectionSourceV1::Private::s_listener = {
    sendCallback,
    cancelledCallback,
};

PrimarySelectionSourceV1::Private::Private(PrimarySelectionSourceV1 *q)
    : q(q)
{
}

void PrimarySelectionSourceV1::Private::sendCallback(void *data, zwp_primary_selection_source_v1 *primarySelectSource, const char *mimeType, int32_t fd)
{
    Q_ASSERT(data);
    auto d = reinterpret_cast<PrimarySelectionSourceV1::Private*>(data);
    Q_ASSERT(d->source == primarySelectSource);
    emit d->q->sendDataRequested(QString::fromUtf8(mimeType), fd);
}

void PrimarySelectionSourceV1::Private::cancelledCallback(void *data, zwp_primary_selection_source_v1 *primarySelectSource)
{
    auto d = reinterpret_cast<PrimarySelectionSourceV1::Private*>(data);
    Q_ASSERT(d->source == primarySelectSource);
    emit d->q->cancelled();
}


void PrimarySelectionSourceV1::Private::setup(zwp_primary_selection_source_v1 *s)
{
    Q_ASSERT(!source.isValid());
    Q_ASSERT(s);
    source.setup(s);
    zwp_primary_selection_source_v1_add_listener(s, &s_listener, this);
}

PrimarySelectionSourceV1::PrimarySelectionSourceV1(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

PrimarySelectionSourceV1::~PrimarySelectionSourceV1()
{
    release();
}

void PrimarySelectionSourceV1::release()
{
    d->source.release();
}

void PrimarySelectionSourceV1::destroy()
{
    d->source.destroy();
}

bool PrimarySelectionSourceV1::isValid() const
{
    return d->source.isValid();
}

void PrimarySelectionSourceV1::setup(zwp_primary_selection_source_v1 *dataSource)
{
    if (dataSource)
        d->setup(dataSource);
}

void PrimarySelectionSourceV1::offer(const QString &mimeType)
{
    zwp_primary_selection_source_v1_offer(d->source, mimeType.toUtf8().constData());
}

void PrimarySelectionSourceV1::offer(const QMimeType &mimeType)
{
    if (!mimeType.isValid()) {
        return;
    }
    offer(mimeType.name());
}

PrimarySelectionSourceV1::operator zwp_primary_selection_source_v1*() const
{
    return d->source;
}

PrimarySelectionSourceV1::operator zwp_primary_selection_source_v1*()
{
    return d->source;
}

}
}
