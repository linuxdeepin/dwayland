/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_SERVER_PRIMARY_SELECTION_SOURCE_INTERFACE_H
#define WAYLAND_SERVER_PRIMARY_SELECTION_SOURCE_INTERFACE_H


#include <KWayland/Server/kwaylandserver_export.h>
#include "abstract_data_source.h"
#include "primaryselectiondevicemanager_v1_interface.h"

namespace KWayland
{
namespace Server
{

class PrimarySelectionSourceV1InterfacePrivate;

/**
 * @brief Represents the Resource for the zwp_primary_selection_source_v1 interface.
 * Lifespan is mapped to the underlying object
 **/
class KWAYLANDSERVER_EXPORT PrimarySelectionSourceV1Interface : public AbstractDataSource
{
    Q_OBJECT
public:
    virtual  ~PrimarySelectionSourceV1Interface() ;

    void requestData(const QString &mimeType, qint32 fd) override;
    void cancel() override;

    QStringList mimeTypes() const override;

    static PrimarySelectionSourceV1Interface *get(wl_resource *native);
private:
    class Private;
    friend class PrimarySelectionDeviceManagerV1Interface;
    explicit PrimarySelectionSourceV1Interface(PrimarySelectionDeviceManagerV1Interface *parent, ::wl_resource *resource);

   Private *d_func() const;
};

}
}

Q_DECLARE_METATYPE(KWayland::Server::PrimarySelectionSourceV1Interface*)

#endif
