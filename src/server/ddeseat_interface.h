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
#ifndef WAYLAND_SERVER_DDE_SEAT_INTERFACE_H
#define WAYLAND_SERVER_DDE_SEAT_INTERFACE_H

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
class DDEPointerInterface;
class DDEKeyboardInterface;
class DDETouchInterface;

/** @class DDESeatInterface
 *
 *
 * @see DDESeatInterface
 * @since 5.5
 */
class KWAYLANDSERVER_EXPORT DDESeatInterface : public Global
{
    Q_OBJECT
public:
    virtual ~DDESeatInterface();

    static DDESeatInterface *get(wl_resource *native);

    /**
     * Updates the global pointer @p pos.
     *
     **/
    void setPointerPos(const QPointF &pos);
    /**
     * @returns the global pointer position
     **/
    QPointF pointerPos() const;
    /**
     * Marks the @p button as pressed.
     *
     * If there is a focused pointer surface a button pressed event is sent to it.
     *
     * @param button The Linux button code
     **/
    void pointerButtonPressed(quint32 button);
    /**
     * Marks the @p button as released.
     *
     * If there is a focused pointer surface a button release event is sent to it.
     *
     * @param button The Linux button code
     **/
    void pointerButtonReleased(quint32 button);

    void pointerAxis(Qt::Orientation orientation, qint32 delta);

    void setTimestamp(quint32 time);
    void setTouchTimestamp(quint32 time);
    quint32 timestamp() const;
    quint32 touchtimestamp() const;

    void setKeymap(int fd, quint32 size);
    void keyPressed(quint32 key);
    void keyReleased(quint32 key);
    void updateKeyboardModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group);

    quint32 depressedModifiers() const;
    quint32 latchedModifiers() const;
    quint32 lockedModifiers() const;
    quint32 groupModifiers() const;
    quint32 lastModifiersSerial() const;

    void touchDown(qint32 id, const QPointF &pos);
    void touchMotion(qint32 id, const QPointF &pos);
    void touchUp(qint32 id);

Q_SIGNALS:
    /**
     * Emitted whenever a DDEPointerInterface got created.
     **/
    void ddePointerCreated(KWayland::Server::DDEPointerInterface*);
    void pointerPosChanged(const QPointF &pos);

    void ddeKeyboardCreated(KWayland::Server::DDEKeyboardInterface*);

    void ddeTouchCreated(KWayland::Server::DDETouchInterface*);

private:
    friend class Display;
    explicit DDESeatInterface(Display *display, QObject *parent = nullptr);
    QPointF globalPos;
    class Private;
    Private *d_func() const;
};

/**
 * @brief Resource for the dde_pointer interface.
 *
 * DDEPointerInterface gets created by DDESeatInterface.
 *
 * @since 5.4
 **/
class KWAYLANDSERVER_EXPORT DDEPointerInterface : public Resource
{
    Q_OBJECT
public:
    virtual ~DDEPointerInterface();

    /**
     * @returns The DDESeatInterface which created this DDEPointerInterface.
     **/
    DDESeatInterface *ddeSeat() const;

    /**
     * @returns The DDEPointerInterface for the @p native resource.
     * @since 5.5
     **/
    static DDEPointerInterface *get(wl_resource *native);

Q_SIGNALS:
    void getMotionRequested();

private:
    friend class DDESeatInterface;
    void buttonPressed(quint32 button);
    void buttonReleased(quint32 button);
    void axis(Qt::Orientation orientation, qint32 delta);
    explicit DDEPointerInterface(DDESeatInterface *ddeSeat, wl_resource *parentResource);
    class Private;
    Private *d_func() const;
};

/**
 * @brief Resource for the dde_touch interface.
 *
 * DDEPointerInterface gets created by DDESeatInterface.
 *
 * @since 5.4
 **/
class KWAYLANDSERVER_EXPORT DDETouchInterface : public Resource
{
    Q_OBJECT
public:
    virtual ~DDETouchInterface();

    /**
     * @returns The DDESeatInterface which created this DDETouchInterface.
     **/
    DDESeatInterface *ddeSeat() const;

    /**
     * @returns The DDETouchInterface for the @p native resource.
     * @since 5.5
     **/
    static DDETouchInterface *get(wl_resource *native);

private:
    friend class DDESeatInterface;

    void touchDown(qint32 id, const QPointF &pos);
    void touchMotion(qint32 id, const QPointF &pos);
    void touchUp(qint32 id);
    explicit DDETouchInterface(DDESeatInterface *ddeSeat, wl_resource *parentResource);
    class Private;
    Private *d_func() const;
};

}
}

#endif
