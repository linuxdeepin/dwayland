// Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
// SPDX-FileCopyrightText: 2022 Martin Gräßlin <mgraesslin@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "primaryselectiondevice_v1.h"
#include "primaryselectionoffer_v1.h"
#include "primaryselectionsource_v1.h"
#include "surface.h"
#include "wayland_pointer_p.h"
// Qt
#include <QPointer>
// Wayland
#include "wayland-wp-primary-selection-unstable-v1-client-protocol.h"


namespace KWayland
{
namespace Client
{

class Q_DECL_HIDDEN PrimarySelectionDeviceV1::Private
{
public:
    explicit Private(PrimarySelectionDeviceV1 *q);
    void setup(zwp_primary_selection_device_v1 *d);

    WaylandPointer<zwp_primary_selection_device_v1, zwp_primary_selection_device_v1_destroy> device;
    QScopedPointer<PrimarySelectionOfferV1> primarySelectionOffer;
private:
    void primarySelectOffer(zwp_primary_selection_offer_v1 *id);
    void selection(zwp_primary_selection_offer_v1 *id);
    static void primarySelectOfferCallback(void *data, zwp_primary_selection_device_v1 *primarySelectDevice, zwp_primary_selection_offer_v1 *id);
    static void selectionCallback(void *data, zwp_primary_selection_device_v1 *primarySelectDevice, zwp_primary_selection_offer_v1 *id);

    static const struct zwp_primary_selection_device_v1_listener s_listener;

    PrimarySelectionDeviceV1 *q;
    PrimarySelectionOfferV1 *lastOffer = nullptr;
};

const zwp_primary_selection_device_v1_listener PrimarySelectionDeviceV1::Private::s_listener = {
    primarySelectOfferCallback,
    selectionCallback
};

void PrimarySelectionDeviceV1::Private::primarySelectOfferCallback(void *data, zwp_primary_selection_device_v1 *primarySelectDevice, zwp_primary_selection_offer_v1 *id)
{
    Q_ASSERT(data);
    auto d = reinterpret_cast<Private*>(data);
    Q_ASSERT(d->device == primarySelectDevice);
    d->primarySelectOffer(id);
}

void PrimarySelectionDeviceV1::Private::primarySelectOffer(zwp_primary_selection_offer_v1 *id)
{
    Q_ASSERT(!lastOffer);
    lastOffer = new PrimarySelectionOfferV1(q, id);
    Q_ASSERT(lastOffer->isValid());
}

void PrimarySelectionDeviceV1::Private::selectionCallback(void *data, zwp_primary_selection_device_v1 *primarySelectDevice, zwp_primary_selection_offer_v1 *id)
{
    auto d = reinterpret_cast<Private*>(data);
    Q_ASSERT(d->device == primarySelectDevice);
    d->selection(id);
}

void PrimarySelectionDeviceV1::Private::selection(zwp_primary_selection_offer_v1 *id)
{
    if (!id) {
        primarySelectionOffer.reset();
        emit q->selectionCleared();
        return;
    }
    Q_ASSERT(*lastOffer == id);
    primarySelectionOffer.reset(lastOffer);
    lastOffer = nullptr;
    emit q->selectionOffered(primarySelectionOffer.data());
}

PrimarySelectionDeviceV1::Private::Private(PrimarySelectionDeviceV1 *q)
    : q(q)
{
}

void PrimarySelectionDeviceV1::Private::setup(zwp_primary_selection_device_v1 *d)
{
    Q_ASSERT(d);
    Q_ASSERT(!device.isValid());
    device.setup(d);
    zwp_primary_selection_device_v1_add_listener(device, &s_listener, this);
}

PrimarySelectionDeviceV1::PrimarySelectionDeviceV1(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

PrimarySelectionDeviceV1::~PrimarySelectionDeviceV1()
{
    release();
}

void PrimarySelectionDeviceV1::destroy()
{
    d->device.destroy();
}

void PrimarySelectionDeviceV1::release()
{
    d->device.release();
}

bool PrimarySelectionDeviceV1::isValid() const
{
    return d->device.isValid();
}

void PrimarySelectionDeviceV1::setup(zwp_primary_selection_device_v1 *primarySelectDevice)
{
    d->setup(primarySelectDevice);
}

namespace {
static zwp_primary_selection_source_v1 *primarySelectSource(const PrimarySelectionSourceV1 *source)
{
    if (!source) {
        return nullptr;
    }
    return *source;
}
}


void PrimarySelectionDeviceV1::setSelection(quint32 serial, PrimarySelectionSourceV1 *source)
{
    zwp_primary_selection_device_v1_set_selection(d->device, primarySelectSource(source), serial);
}

void PrimarySelectionDeviceV1::clearSelection(quint32 serial)
{
    setSelection(serial);
}

PrimarySelectionOfferV1 *PrimarySelectionDeviceV1::offeredSelection() const
{
    return d->primarySelectionOffer.data();
}

PrimarySelectionDeviceV1::operator zwp_primary_selection_device_v1*()
{
    return d->device;
}

PrimarySelectionDeviceV1::operator zwp_primary_selection_device_v1*() const
{
    return d->device;
}

}
}
