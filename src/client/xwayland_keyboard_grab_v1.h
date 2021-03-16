/********************************************************************
Copyright 2021  zhangyaning <zhangyaning@uniontech.com>

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
#ifndef WAYLAND_xwayland_keyboard_grab_v1_H
#define WAYLAND_xwayland_keyboard_grab_v1_H

#include <QObject>
#include <QPointer>
#include <QSize>
#include <QVector>

#include <KWayland/Client/kwaylandclient_export.h>

struct zwp_xwayland_keyboard_grab_manager_v1;
struct zwp_xwayland_keyboard_grab_v1;
class  QPoint;
class  QRect;

namespace KWayland
{
namespace Client
{

class EventQueue;
class ZWPXwaylandKeyboardGrabV1;
class Surface;
class Seat;

/**
 * @short Wrapper for the zwp_xwayland_keyboard_grab_manager_v1 interface.
 *
 * This class provides a convenient wrapper for the zwp_xwayland_keyboard_grab_manager_v1 interface.
 * Its main purpose is to hold the information about one ZWPXwaylandKeyboardGrabManagerV1.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create an ZWPXwaylandKeyboardGrabManagerV1 interface:
 * @code
 * ZWPXwaylandKeyboardGrabManagerV1 *c = registry->createDDESeat(name, version);
 * @endcode
 *
 * This creates the ZWPXwaylandKeyboardGrabManagerV1 and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * ZWPXwaylandKeyboardGrabManagerV1 *c = new ZWPXwaylandKeyboardGrabManagerV1;
 * c->setup(registry->bindDDESeat(name, version));
 * @endcode
 *
 * The ZWPXwaylandKeyboardGrabManagerV1 can be used as a drop-in replacement for any zwp_xwayland_keyboard_grab_manager_v1
 * pointer as it provides matching cast operators.
 *
 * Please note that all properties of ZWPXwaylandKeyboardGrabManagerV1 are not valid until the
 * changed signal has been emitted. The wayland server is pushing the
 * information in an async way to the ZWPXwaylandKeyboardGrabManagerV1 instance. By emitting changed
 * the ZWPXwaylandKeyboardGrabManagerV1 indicates that all relevant information is available.
 *
 * @see Registry
 * @since 5.5
 **/
class KWAYLANDCLIENT_EXPORT ZWPXwaylandKeyboardGrabManagerV1 : public QObject
{
    Q_OBJECT
public:
    enum GrabType {
        GrabServer,
        GrabKeyboard,
        GrabPointer
    };

    explicit ZWPXwaylandKeyboardGrabManagerV1(QObject *parent = nullptr);
    virtual ~ZWPXwaylandKeyboardGrabManagerV1();

    /**
     * Setup this Compositor to manage the @p ZWPXwaylandKeyboardGrabManagerV1.
     * When using Registry::createDDESeat there is no need to call this
     * method.
     **/
    void setup(zwp_xwayland_keyboard_grab_manager_v1 *ZWPXwaylandKeyboardGrabManagerV1);

    /**
     * @returns @c true if managing a zwp_xwayland_keyboard_grab_manager_v1.
     **/
    bool isValid() const;
    operator zwp_xwayland_keyboard_grab_manager_v1*();
    operator zwp_xwayland_keyboard_grab_manager_v1*() const;

    /**
     * Creates a ZWPXwaylandKeyboardGrabV1 and sets it up.
     *
     *
     * @param parent The parent to use for the ZWPXwaylandKeyboardGrabV1
     * @returns created ZWPXwaylandKeyboardGrabV1
     **/
    ZWPXwaylandKeyboardGrabV1 *grabKeyBoard(Surface *surface, Seat *seat, QObject *parent = nullptr);

    /**
     * Sets the @p queue to use for bound proxies.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for bound proxies.
     **/
    EventQueue *eventQueue() const;

    void release();
    /**
    * Destroys the data hold by this ZWPXwaylandKeyboardGrabManagerV1.
    * This method is supposed to be used when the connection to the Wayland
    * server goes away. If the connection is not valid any more, it's not
    * possible to call release any more as that calls into the Wayland
    * connection and the call would fail.
    *
    * This method is automatically invoked when the Registry which created this
    * ZWPXwaylandKeyboardGrabManagerV1 gets destroyed.
    **/
    void destroy();

Q_SIGNALS:
    /**
     * This signal is emitted right before the interface is released.
     **/
    void interfaceAboutToBeReleased();
    /**
     * This signal is emitted right before the data is destroyed.
     **/
    void interfaceAboutToBeDestroyed();

    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the ZWPXwaylandKeyboardGrabManagerV1 got created by
     * Registry::createDDESeat
     *
     * @since 5.5
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

/**
 * @short Wrapper for the zwp_xwayland_keyboard_grab_v1 interface.
 *
 * This class is a convenient wrapper for the zwp_xwayland_keyboard_grab_v1 interface.
 *
 * To create an instance use ZWPXwaylandKeyboardGrabManagerV1::createDDePointer.
 *
 *
 * @see ZWPXwaylandKeyboardGrabManagerV1
 **/
class KWAYLANDCLIENT_EXPORT ZWPXwaylandKeyboardGrabV1 : public QObject
{
    Q_OBJECT
public:

    explicit ZWPXwaylandKeyboardGrabV1(QObject *parent);
    virtual ~ZWPXwaylandKeyboardGrabV1();

    /**
     * Releases the zwp_xwayland_keyboard_grab_v1 interface.
     * After the interface has been released the ZWPXwaylandKeyboardGrabV1 instance is no
     * longer valid and can be setup with another zwp_xwayland_keyboard_grab_v1 interface.
     *
     * This method is automatically invoked when the ZWPXwaylandKeyboardGrabManagerV1 which created this
     * ZWPXwaylandKeyboardGrabV1 gets released.
     **/
    void release();
    /**
     * Destroys the data held by this ZWPXwaylandKeyboardGrabV1.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new zwp_xwayland_keyboard_grab_v1 interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the ZWPXwaylandKeyboardGrabManagerV1 which created this
     * ZWPXwaylandKeyboardGrabV1 gets destroyed.
     *
     * @see release
     **/
    void destroy();
    /**
     * Setup this ZWPXwaylandKeyboardGrabV1 to manage the @p zwp_xwayland_keyboard_grab_v1.
     * There is normally no need to call this method as it's invoked by
     * ZWPXwaylandKeyboardGrabManagerV1::createDDEPointer.
     **/
    void setup(zwp_xwayland_keyboard_grab_v1 *grab);

    /**
     * @returns @c true if managing a zwp_xwayland_keyboard_grab_v1.
     **/
    bool isValid() const;
    operator zwp_xwayland_keyboard_grab_v1*();
    operator zwp_xwayland_keyboard_grab_v1*() const;


private:
    friend class ZWPXwaylandKeyboardGrabManagerV1;
    class Private;
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(KWayland::Client::ZWPXwaylandKeyboardGrabManagerV1::GrabType)

#endif
