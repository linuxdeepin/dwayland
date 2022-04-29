/********************************************************************
Copyright 2020  Zhang Liang <zhanglianga@uniontech.com>

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
    Q_EMIT interfaceAboutToBeReleased();
    d->strut.release();
}

void Strut::destroy()
{
    if (!d->strut) {
        return;
    }
    Q_EMIT interfaceAboutToBeDestroyed();
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
