// Copyright 2020  Zhang Liang <zhanglianga@uniontech.com>
// SPDX-FileCopyrightText: 2022 Martin Gräßlin <mgraesslin@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "strut.h"
#include "event_queue.h"
#include "wayland_pointer_p.h"
#include <wayland-strut-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN Strut::Private
{
public:
    Private() = default;
    WaylandPointer<com_deepin_kwin_strut, com_deepin_kwin_strut_destroy> strut;
    EventQueue *queue = nullptr;
};

Strut::Strut(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

Strut::~Strut()
{
    release();
}

void Strut::setup(com_deepin_kwin_strut *strut)
{
    Q_ASSERT(strut);
    Q_ASSERT(!d->strut);
    d->strut.setup(strut);
}

bool Strut::isValid() const
{
    if (!d->strut) {
        return false;
    }
    return d->strut.isValid();
}

void Strut::release()
{
    if (!d->strut) {
        return;
    }
    emit interfaceAboutToBeReleased();
    d->strut.release();
}

void Strut::destroy()
{
    if (!d->strut) {
        return;
    }
    emit interfaceAboutToBeDestroyed();
    d->strut.destroy();
}

void Strut::setEventQueue(EventQueue *queue)
{
    if (d->queue) {
        d->queue = queue;
    }
}

EventQueue *Strut::eventQueue()
{
    return d->queue;
}

void Strut::setStrutPartial(wl_surface *surface,struct deepinKwinStrut& sStrut)
{
    if (!d->strut || !surface) {
        return;
    }
    com_deepin_kwin_strut_set_strut_partial(d->strut, surface,
                                            sStrut.left,sStrut.right,sStrut.top,sStrut.bottom,
                                            sStrut.left_start_y,sStrut.left_end_y,sStrut.right_start_y,sStrut.right_end_y,
                                            sStrut.top_start_x,sStrut.top_end_x,sStrut.bottom_start_x,sStrut.bottom_end_x);
}

Strut::operator com_deepin_kwin_strut*()
{
    return d->strut;
}

Strut::operator com_deepin_kwin_strut*() const
{
    return d->strut;
}

}
}
