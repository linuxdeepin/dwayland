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
#ifndef WAYLAND_DDE_SHELL_H
#define WAYLAND_DDE_SHELL_H

#include <QObject>
#include <QPointer>
#include <QSize>
#include <QRect>
#include <QVector>
#include <QColor>

#include <KWayland/Client/kwaylandclient_export.h>

struct dde_shell;
struct dde_shell_surface;
struct wl_surface;
class QPoint;
class QRect;

namespace KWayland
{
namespace Client
{

class EventQueue;
class Surface;
class DDEShellSurface;

/**
 * @short Wrapper for the dde_shell interface.
 *
 * This class provides a convenient wrapper for the dde_shell interface.
 * Its main purpose is to hold the information about one DDEShell.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create an DDEShell interface:
 * @code
 * DDEShell *c = registry->createDDEShell(name, version);
 * @endcode
 *
 * This creates the DDEShell and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * DDEShell *c = new DDEShell;
 * c->setup(registry->bindDDEShell(name, version));
 * @endcode
 *
 * The DDEShell can be used as a drop-in replacement for any dde_shell
 * pointer as it provides matching cast operators.
 *
 * Please note that all properties of DDEShell are not valid until the
 * changed signal has been emitted. The wayland server is pushing the
 * information in an async way to the DDEShell instance. By emitting changed
 * the DDEShell indicates that all relevant information is available.
 *
 * @see Registry
 * @since 5.5
 **/
class KWAYLANDCLIENT_EXPORT DDEShell : public QObject
{
    Q_OBJECT
public:

    explicit DDEShell(QObject *parent = nullptr);
    virtual ~DDEShell();

    /**
     * Setup this Compositor to manage the @p DDEShell.
     * When using Registry::createDDEShell there is no need to call this
     * method.
     **/
    void setup(dde_shell *DDEShell);

    /**
     * @returns @c true if managing a dde_shell.
     **/
    bool isValid() const;
    operator dde_shell*();
    operator dde_shell*() const;
    dde_shell *ddeShell();

    /**
     * Creates a DDEShellSurface for the given @p surface and sets it up.
     *
     * If a DDEShellSurface for the given @p surface has already been created
     * a pointer to the existing one is returned instead of creating a new surface.
     *
     * @param surface The native surface to create the DDEShellSurface for
     * @param parent The parent to use for the DDEShellSurface
     * @returns created DDEShellSurface
     **/
    DDEShellSurface *createShellSurface(wl_surface *surface, QObject *parent = nullptr);
    /**
     * Creates a DDEShellSurface for the given @p surface and sets it up.
     *
     * If a DDEShellSurface for the given @p surface has already been created
     * a pointer to the existing one is returned instead of creating a new surface.
     *
     * @param surface The Surface to create the DDEShellSurface for
     * @param parent The parent to use for the DDEShellSurface
     * @returns created DDEShellSurface
     **/
    DDEShellSurface *createShellSurface(Surface *surface, QObject *parent = nullptr);

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
    * Destroys the data hold by this DDEShell.
    * This method is supposed to be used when the connection to the Wayland
    * server goes away. If the connection is not valid any more, it's not
    * possible to call release any more as that calls into the Wayland
    * connection and the call would fail.
    *
    * This method is automatically invoked when the Registry which created this
    * DDEShell gets destroyed.
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
     * This signal gets only emitted if the DDEShell got created by
     * Registry::createDDEShell
     *
     * @since 5.5
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};


class KWAYLANDCLIENT_EXPORT DDEShellSurface : public QObject
{
    Q_OBJECT
public:

    explicit DDEShellSurface(QObject *parent = nullptr);
    virtual ~DDEShellSurface();

    void setup(dde_shell_surface *ddeShellSuface);

    bool isValid() const;
    operator dde_shell_surface*();
    operator dde_shell_surface*() const;

    void release();
    void destroy();

    static DDEShellSurface *get(wl_surface *surf);
    static DDEShellSurface *get(Surface *surf);

    bool isActive() const;
    bool isFullscreen() const;
    bool isKeepAbove() const;
    bool isKeepBelow() const;
    bool isMinimized() const;
    bool isMaximized() const;
    bool isCloseable() const;
    bool isMaximizeable() const;
    bool isMinimizeable() const;
    bool isFullscreenable() const;
    bool isMovable() const;
    bool isResizable() const;
    bool isAcceptFocus() const;
    bool isModal() const;
    bool isSplitable() const;
    bool isOnAllDesktops() const;

    void requestActivate();
    void requestKeepAbove(bool set);
    void requestKeepBelow(bool set);
    void requestMinimized(bool set);
    void requestMaximized(bool set);
    void requestAcceptFocus(bool set);
    void requestModal(bool set);
    void requestMinizeable(bool set);
    void requestMaximizeable(bool set);
    void requestResizable(bool set);
    void requestOnAllDesktops(bool set);

    void requestGeometry() const;
    void requestActive() const;
    QRect getGeometry() const;

    void requestNoTitleBarProperty(qint32 value);
    void requestWindowRadiusProperty(QPointF windowRadius);

    enum class SplitType {
        leftSplit        = 1 << 0,
        rightSplit       = 1 << 1,
    };

    void requestSplitWindow(SplitType splitType);
Q_SIGNALS:
    void geometryChanged(const QRect &geom);
    void activeChanged();
    void fullscreenChanged();
    void keepAboveChanged();
    void keepBelowChanged();
    void minimizedChanged();
    void maximizedChanged();
    void closeableChanged();
    void minimizeableChanged();
    void maximizeableChanged();
    void fullscreenableChanged();
    void movableChanged();
    void resizableChanged();
    void acceptFocusChanged();
    void modalityChanged();
    void onAllDesktopsChanged();

private:
    friend class DDEShell;
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
