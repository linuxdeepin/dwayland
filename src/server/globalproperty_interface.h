/********************************************************************
Copyright 2022  diguoliang <diguoliang@uniontech.com>

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
#pragma once

#include <QObject>
#include <QString>
#include <QMap>

#include <DWayland/Server/kwaylandserver_export.h>

struct wl_resource;

namespace KWaylandServer
{
class Display;
class wl_surface;
class SurfaceInterface;;
class GlobalPropertyInterfacePrivate;

struct PropertyData{
    QString module;
    QString function;
    QMap<QString, QVariant> propetyData;
};

class KWAYLANDSERVER_EXPORT GlobalPropertyInterface : public QObject
{
    Q_OBJECT
    //Q_PROPERTY(QString propertyData READ propertyData WRITE setPropertyData NOTIFY WindowDecoratePropertyChanged)
public:
    explicit GlobalPropertyInterface(Display *display, QObject *parent = nullptr);
    virtual ~GlobalPropertyInterface();

Q_SIGNALS:
    void windowDecoratePropertyChanged(SurfaceInterface *, QMap<QString, QVariant> &) const;

private:
    QScopedPointer<GlobalPropertyInterfacePrivate> d;
};

}