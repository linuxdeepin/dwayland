/********************************************************************
Copyright 2020  wugang <wugang@uniontech.com>

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
#ifndef WAYLAND_SERVER_DDE_SHELL_INTERFACE_H
#define WAYLAND_SERVER_DDE_SHELL_INTERFACE_H

#include <QObject>
#include <QPoint>
#include <QSize>
#include <QRect>
#include <QVector>

#include <KWayland/Server/kwaylandserver_export.h>
#include "global.h"
#include "resource.h"

struct wl_resource;

namespace KWayland
{
namespace Server
{

class Display;
class SurfaceInterface;
class DDEShellInterface;
class DDEShellSurfaceInterface;

class KWAYLANDSERVER_EXPORT DDEShellInterface : public Global
{
    Q_OBJECT
public:
    virtual ~DDEShellInterface();

    static DDEShellInterface *get(wl_resource *native);

Q_SIGNALS:
    void shellSurfaceCreated(KWayland::Server::DDEShellSurfaceInterface*);
private:
    friend class Display;
    explicit DDEShellInterface(Display *display, QObject *parent = nullptr);
    class Private;
    Private *d_func() const;
};

class KWAYLANDSERVER_EXPORT DDEShellSurfaceInterface : public Resource
{
    Q_OBJECT
public:
    virtual ~DDEShellSurfaceInterface();

    DDEShellInterface *ddeShell() const;
    SurfaceInterface *surface() const;

    static DDEShellSurfaceInterface *get(wl_resource *native);
    void sendGeometry(const QRect &geom);
    void sendSplitable(bool splitable);

    void setActive(bool set);
    void setMinimized(bool set);
    void setMaximized(bool set);
    void setFullscreen(bool set);
    void setKeepAbove(bool set);
    void setKeepBelow(bool set);
    void setOnAllDesktops(bool set);
    void setCloseable(bool set);
    void setMinimizeable(bool set);
    void setMaximizeable(bool set);
    void setFullscreenable(bool set);
    void setMovable(bool set);
    void setResizable(bool set);
    void setAcceptFocus(bool set);
    void setModal(bool set);

    enum class SplitType {
        leftSplit        = 1 << 0,
        rightSplit       = 1 << 1,
    };

Q_SIGNALS:
    void activationRequested();
    void activeRequested(bool set);
    void minimizedRequested(bool set);
    void maximizedRequested(bool set);
    void fullscreenRequested(bool set);
    void keepAboveRequested(bool set);
    void keepBelowRequested(bool set);
    void onAllDesktopsRequested(bool set);
    void closeableRequested(bool set);
    void minimizeableRequested(bool set);
    void maximizeableRequested(bool set);
    void fullscreenableRequested(bool set);
    void movableRequested(bool set);
    void resizableRequested(bool set);
    void acceptFocusRequested(bool set);
    void modalityRequested(bool set);

    void noTitleBarPropertyRequested(qint32 value);
    void windowRadiusPropertyRequested(QPointF windowRadius);
    void splitWindowRequested(SplitType splitType);

private:
    friend class DDEShellInterface;
    QRect geometry;
    explicit DDEShellSurfaceInterface(DDEShellInterface *ddeShell, SurfaceInterface *parent, wl_resource *parentResource);
    class Private;
    Private *d_func() const;
};

}
}

#endif
