// Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
// SPDX-FileCopyrightText: 2022 Martin Gräßlin <mgraesslin@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "primaryselectiondevicemanager_v1.h"
#include "primaryselectiondevice_v1.h"
#include "primaryselectionsource_v1.h"
#include "event_queue.h"
#include "seat.h"
#include "wayland_pointer_p.h"

#include "wayland-wp-primary-selection-unstable-v1-client-protocol.h"

namespace KWayland
{
namespace Client
{

class Q_DECL_HIDDEN PrimarySelectionDeviceManagerV1::Private
{
public:
    WaylandPointer<zwp_primary_selection_device_manager_v1, zwp_primary_selection_device_manager_v1_destroy> manager;
    EventQueue *queue = nullptr;
};

PrimarySelectionDeviceManagerV1::PrimarySelectionDeviceManagerV1(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

PrimarySelectionDeviceManagerV1::~PrimarySelectionDeviceManagerV1()
{
    release();
}

void PrimarySelectionDeviceManagerV1::release()
{
    d->manager.release();
}

void PrimarySelectionDeviceManagerV1::destroy()
{
    d->manager.destroy();
}

bool PrimarySelectionDeviceManagerV1::isValid() const
{
    return d->manager.isValid();
}

void PrimarySelectionDeviceManagerV1::setup(zwp_primary_selection_device_manager_v1 *manager)
{
    Q_ASSERT(manager);
    Q_ASSERT(!d->manager.isValid());
    d->manager.setup(manager);
}

EventQueue *PrimarySelectionDeviceManagerV1::eventQueue()
{
    return d->queue;
}

void PrimarySelectionDeviceManagerV1::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

PrimarySelectionSourceV1 *PrimarySelectionDeviceManagerV1::createPrimarySelectSource(QObject *parent)
{
    Q_ASSERT(isValid());
    PrimarySelectionSourceV1 *s = new PrimarySelectionSourceV1(parent);
    auto w = zwp_primary_selection_device_manager_v1_create_source(d->manager);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

PrimarySelectionDeviceV1 *PrimarySelectionDeviceManagerV1::getPrimarySelectDevice(Seat *seat, QObject *parent)
{
    Q_ASSERT(isValid());
    Q_ASSERT(seat);
    PrimarySelectionDeviceV1 *device = new PrimarySelectionDeviceV1(parent);
    auto w = zwp_primary_selection_device_manager_v1_get_device(d->manager, *seat);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    device->setup(w);
    return device;
}

PrimarySelectionDeviceManagerV1::operator zwp_primary_selection_device_manager_v1*() const
{
    return d->manager;
}

PrimarySelectionDeviceManagerV1::operator zwp_primary_selection_device_manager_v1*()
{
    return d->manager;
}

}
}
