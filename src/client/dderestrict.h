/********************************************************************
Copyright 2020  pengwenhao <pengwenhao@uniontech.com>

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
#ifndef WAYLAND_DDE_RESTRICT_H
#define WAYLAND_DDE_RESTRICT_H

#include <QObject>
#include <QPointer>
#include <QSize>
#include <QRect>
#include <QVector>

#include <KWayland/Client/kwaylandclient_export.h>

struct dde_restrict;
struct wl_surface;

namespace KWayland
{
namespace Client
{

class EventQueue;

/**
 * @short Wrapper for the dde_restrict interface.
 *
 * This class provides a convenient wrapper for the dde_restrict interface.
 * Its main purpose is to hold the information about one DDERestrict.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create an DDERestrict interface:
 * @code
 * DDEShell *c = registry->createDDERestrict(name, version);
 * @endcode
 *
 * This creates the DDERestrict and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * DDEShell *c = new DDRestrict;
 * c->setup(registry->bindDDERestrict(name, version));
 * @endcode
 *
 * The DDERestrict can be used as a drop-in replacement for any dde_restrict
 * pointer as it provides matching cast operators.
 *
 * Please note that all properties of DDERestrict are not valid until the
 * changed signal has been emitted. The wayland server is pushing the
 * information in an async way to the DDERestrict instance. By emitting changed
 * the DDERestrict indicates that all relevant information is available.
 *
 * @see Registry
 * @since 5.5
 **/
class KWAYLANDCLIENT_EXPORT DDERestrict : public QObject
{
    Q_OBJECT
public:
    enum SwitchFlag {
        On = 1,
        Off = 2
    };

    Q_DECLARE_FLAGS(SwitchFlags, SwitchFlag)

    explicit DDERestrict(QObject *parent = nullptr);
    virtual ~DDERestrict();

    /**
     * Setup this Compositor to manage the @p DDEShell.
     * When using Registry::createDDERestrict there is no need to call this
     * method.
     **/
    void setup(dde_restrict *manager);

    /**
     * @returns @c true if managing a dde_restrict.
     **/
    bool isValid() const;

   /**
    * Releases the dde_restrict interface.
    * After the interface has been released the Idle instance is no
    * longer valid and can be setup with another dde_restrict interface.
    **/
    void release();

    /**
    * Destroys the data hold by this DDERestrict.
    * This method is supposed to be used when the connection to the Wayland
    * server goes away. If the connection is not valid any more, it's not
    * possible to call release any more as that calls into the Wayland
    * connection and the call would fail.
    *
    * This method is automatically invoked when the Registry which created this
    * DDEShell gets destroyed.
    **/
    void destroy();

    /**
     * Sets the @p queue to use for bound proxies.
     **/
    void setEventQueue(EventQueue *queue);

    /**
     * @returns The event queue to use for bound proxies.
     **/
    EventQueue *eventQueue() const;

    operator dde_restrict*();
    operator dde_restrict*() const;

    /**
     * Set current screencaast switch
     * On  : screenshots are allowed
     * Off : screenshots are not allowed
     */
    void switchScreencast(SwitchFlags state);

    /**
     * this feature is enabled only when the feature is turned on and the whitelisted client displays
     * whitelists: comma-separated window resource classes
     */
     void setClientWhiteList(const QString& whitelist);

Q_SIGNALS:
    /**
     * Emitted when the user invoked to perform an unauthorised action.
     * @see idle
     **/
    void windowLoged(const QString& info);

    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the DDERestrict got created by
     * Registry::createDDERestrict
     *
     * @since 5.5
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
