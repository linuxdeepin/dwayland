// Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
// SPDX-FileCopyrightText: 2022 Martin Gräßlin <mgraesslin@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

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
