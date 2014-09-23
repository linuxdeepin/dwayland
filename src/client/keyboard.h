/********************************************************************
Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>

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
#ifndef WAYLAND_KEYBOARD_H
#define WAYLAND_KEYBOARD_H

#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

struct wl_keyboard;

namespace KWayland
{
namespace Client
{

/**
 * @short Wrapper for the wl_keyboard interface.
 *
 * This class is a convenient wrapper for the wl_keyboard interface.
 *
 * To create an instance use Seat::createKeyboard.
 *
 * @see Seat
 **/
class KWAYLANDCLIENT_EXPORT Keyboard : public QObject
{
    Q_OBJECT
public:
    enum class KeyState {
        Released,
        Pressed
    };
    explicit Keyboard(QObject *parent = nullptr);
    virtual ~Keyboard();

    /**
     * @returns @c true if managing a wl_keyboard.
     **/
    bool isValid() const;
    /**
     * Setup this Keyboard to manage the @p keyboard.
     * When using Seat::createKeyboard there is no need to call this
     * method.
     **/
    void setup(wl_keyboard *keyboard);
    /**
     * Releases the wl_keyboard interface.
     * After the interface has been released the Keyboard instance is no
     * longer valid and can be setup with another wl_keyboard interface.
     *
     * This method is automatically invoked when the Seat which created this
     * Keyboard gets released.
     **/
    void release();
    /**
     * Destroys the data hold by this Keyboard.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid any more, it's not
     * possible to call release any more as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or setup to a new wl_keyboard interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Seat which created this
     * Keyboard gets destroyed.
     *
     * @see release
     **/
    void destroy();

    operator wl_keyboard*();
    operator wl_keyboard*() const;

Q_SIGNALS:
    /**
     * This signal provides a file descriptor to the client which can
     * be memory-mapped to provide a keyboard mapping description.
     *
     * The signal is only emitted if the keymap format is libxkbcommon compatible.
     *
     * @param fd file descriptor of the keymap
     * @param size The size of the keymap
     **/
    void keymapChanged(int fd, quint32 size);
    /**
     * A key was pressed or released.
     * The time argument is a timestamp with millisecond granularity, with an undefined base.
     * @param key The key which was pressed
     * @param state Whether the key got @c Released or @c Pressed
     * @param time The timestamp
     **/
    void keyChanged(quint32 key, KWayland::Client::Keyboard::KeyState state, quint32 time);
    /**
     * Notifies clients that the modifier and/or group state has changed,
     * and it should update its local state.
     **/
    void modifiersChanged(quint32 depressed, quint32 latched, quint32 locked, quint32 group);

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(KWayland::Client::Keyboard::KeyState)

#endif
