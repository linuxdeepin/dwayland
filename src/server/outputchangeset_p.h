// Copyright 2015 Sebastian Kügler <sebas@kde.org>
// SPDX-FileCopyrightText: 2022 Martin Gräßlin <mgraesslin@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef KWAYLAND_SERVER_OUTPUTCHANGESET_P_H
#define KWAYLAND_SERVER_OUTPUTCHANGESET_P_H

#include "outputchangeset.h"

namespace KWayland
{
    namespace Server
    {

        class OutputChangeSet::Private
        {
        public:
            Private(OutputDeviceInterface *outputdevice, OutputChangeSet *parent);
            ~Private();

            OutputChangeSet *q;
            OutputDeviceInterface *o;

            OutputDeviceInterface::Enablement enabled;
            int modeId;
            int brightness;
            OutputDeviceInterface::Transform transform;
            QPoint position;
            qreal scale;
            OutputDeviceInterface::ColorCurves colorCurves;
        };
    }
}

#endif
