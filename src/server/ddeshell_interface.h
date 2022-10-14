// Copyright 2020  wugang <wugang@uniontech.com>
// SPDX-FileCopyrightText: 2022 Martin Gräßlin <mgraesslin@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

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
