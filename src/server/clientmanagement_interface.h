// Copyright 2020  wugang <wugang@uniontech.com>
// SPDX-FileCopyrightText: 2022 Martin Gräßlin <mgraesslin@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef WAYLAND_SERVER_CLIENT_MANAGEMENT_INTERFACE_H
#define WAYLAND_SERVER_CLIENT_MANAGEMENT_INTERFACE_H

#include <QObject>
#include <QPoint>
#include <QSize>
#include <QVector>
#include <QImage>

#include <KWayland/Server/kwaylandserver_export.h>
#include "global.h"
#include "surface_interface.h"

struct wl_resource;

namespace KWayland
{
namespace Server
{

class Display;

/** @class ClientManagementInterface
 *
 *
 * @see ClientManagementInterface
 * @since 5.5
 */
class KWAYLANDSERVER_EXPORT ClientManagementInterface : public Global
{
    Q_OBJECT
public:
    virtual ~ClientManagementInterface();

    struct WindowState {
        int32_t pid;
        int32_t windowId;
        char resourceName[256];
        struct Geometry {
            int32_t x;
            int32_t y;
            int32_t width;
            int32_t height;
        } geometry;
        bool isMinimized;
        bool isFullScreen;
        bool isActive;
    };

    static ClientManagementInterface *get(wl_resource *native);
    void setWindowStates(QList<WindowState*> &windowStates);

    void sendWindowCaptionImage(int windowId, wl_resource *buffer, QImage image);
    void sendWindowCaption(int windowId, wl_resource *buffer, SurfaceInterface* surface);

Q_SIGNALS:
    void windowStatesRequest();
    void windowStatesChanged();

    void captureWindowImageRequest(int windowId, wl_resource *buffer);

private:
    friend class Display;
    explicit ClientManagementInterface(Display *display, QObject *parent = nullptr);
    class Private;
    Private *d_func() const;
};

}
}

#endif
