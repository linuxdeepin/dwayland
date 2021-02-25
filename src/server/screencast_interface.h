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
#ifndef WAYLAND_SERVER_SCREENCAST_INTERFACE_H
#define WAYLAND_SERVER_SCREENCAST_INTERFACE_H

#include <QObject>
#include <QScopedPointer>

#include <KWayland/Server/kwaylandserver_export.h>


namespace KWayland
{
namespace Server
{

class Display;
class OutputInterface;
class ScreencastV1InterfacePrivate;
class ScreencastStreamV1InterfacePrivate;
class ScreencastStreamV1Interface;

class KWAYLANDSERVER_EXPORT ScreencastStreamV1Interface : public QObject
{
    Q_OBJECT
public:
    ~ScreencastStreamV1Interface() override;

    void sendCreated(quint32 nodeid);
    void sendFailed(const QString &error);
    void sendClosed();

Q_SIGNALS:
    void finished();

private:
    friend class ScreencastV1InterfacePrivate;
    explicit ScreencastStreamV1Interface(QObject *parent);
    QScopedPointer<ScreencastStreamV1InterfacePrivate> d;
};

class KWAYLANDSERVER_EXPORT ScreencastV1Interface : public QObject
{
    Q_OBJECT
public:
    virtual ~ScreencastV1Interface();

    enum CursorMode {
        Hidden = 1,
        Embedded = 2,
        Metadata = 4,
    };
    Q_ENUM(CursorMode);

Q_SIGNALS:
    void outputScreencastRequested(ScreencastStreamV1Interface *stream, OutputInterface *output, CursorMode mode);
    void windowScreencastRequested(ScreencastStreamV1Interface *stream, const QString &winid, CursorMode mode);

private:
    explicit ScreencastV1Interface(Display *display, QObject *parent = nullptr);
    friend class Display;
    QScopedPointer<ScreencastV1InterfacePrivate> d;
};

}
}

#endif
