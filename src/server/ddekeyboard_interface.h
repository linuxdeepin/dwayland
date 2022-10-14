// Copyright 2021  luochaojiang <luochaojiang@uniontech.com>
// SPDX-FileCopyrightText: 2022 Martin Gräßlin <mgraesslin@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef WAYLAND_SERVER_DDE_KEYBOARD_INTERFACE_H
#define WAYLAND_SERVER_DDE_KEYBOARD_INTERFACE_H

#include <QObject>
#include <QPoint>
#include <QSize>
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
class DDESeatInterface;

/**
 * @brief Resource for the dde_keyboard interface.
 *
 * DDEKeyboardInterface gets created by DDESeatInterface.
 *
 * @since 5.4
 **/
class KWAYLANDSERVER_EXPORT DDEKeyboardInterface : public Resource
{
    Q_OBJECT
public:
    explicit DDEKeyboardInterface(DDESeatInterface *ddeSeat, wl_resource *parentResource);
    virtual ~DDEKeyboardInterface();

    /**
     * @returns The DDESeatInterface which created this DDEPointerInterface.
     **/
    DDESeatInterface *ddeSeat() const;

    void setKeymap(int fd, quint32 size);
    void updateModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group, quint32 serial);
    void keyPressed(quint32 key, quint32 serial);
    void keyReleased(quint32 key, quint32 serial);
    void repeatInfo(qint32 charactersPerSecond, qint32 delay);

private:
    class Private;
    Private *d_func() const;
};

}
}

#endif