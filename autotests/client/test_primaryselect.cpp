/********************************************************************
Copyright 2016  Martin Gräßlin <mgraesslin@kde.org>

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
// Qt
#include <QtTest>
// client
#include "../../src/client/connection_thread.h"
#include "../../src/client/compositor.h"
#include "../../src/client/primaryselectiondevice_v1.h"
#include "../../src/client/primaryselectiondevicemanager_v1.h"
#include "../../src/client/primaryselectionsource_v1.h"
#include "../../src/client/event_queue.h"
#include "../../src/client/keyboard.h"
#include "../../src/client/registry.h"
#include "../../src/client/seat.h"
#include "../../src/client/surface.h"
// server
#include "../../src/server/compositor_interface.h"
#include "../../src/server/primaryselectiondevicemanager_v1_interface.h"
#include "../../src/server/display.h"
#include "../../src/server/seat_interface.h"

using namespace KWayland::Client;
using namespace KWayland::Server;


class PrimarySelectionTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init();
    void cleanup();
    void testClearOnEnter();

private:
    Display *m_display = nullptr;
    CompositorInterface *m_compositorInterface = nullptr;
    SeatInterface *m_seatInterface = nullptr;
    PrimarySelectionDeviceManagerV1Interface *m_ddmInterface = nullptr;

    struct Connection {
        ConnectionThread *connection = nullptr;
        QThread *thread = nullptr;
        EventQueue *queue = nullptr;
        Compositor *compositor = nullptr;
        Seat *seat = nullptr;
        PrimarySelectionDeviceManagerV1 *ddm = nullptr;
        Keyboard *keyboard = nullptr;
        PrimarySelectionDeviceV1 *dataDevice = nullptr;
    };
    bool setupConnection(Connection *c);
    void cleanupConnection(Connection *c);

    Connection m_client1;
    Connection m_client2;
};

static const QString s_socketName = QStringLiteral("kwayland-test-selection-0");

void PrimarySelectionTest::init()
{
    delete m_display;
    m_display = new Display(this);
    m_display->setSocketName(s_socketName);
    m_display->start();
    QVERIFY(m_display->isRunning());
    m_display->createShm();
    m_compositorInterface = m_display->createCompositor(m_display);
    m_compositorInterface->create();
    m_seatInterface = m_display->createSeat(m_display);
    m_seatInterface->setHasKeyboard(true);
    m_seatInterface->create();
    m_ddmInterface = m_display->createPrimarySelectionDeviceManagerV1(m_display);
    m_ddmInterface->create();

    // setup connection
    setupConnection(&m_client1);
    setupConnection(&m_client2);
}

bool PrimarySelectionTest::setupConnection(Connection* c)
{
    c->connection = new ConnectionThread;
    QSignalSpy connectedSpy(c->connection, &ConnectionThread::connected);
    if (!connectedSpy.isValid()) {
        return false;
    }
    c->connection->setSocketName(s_socketName);

    c->thread = new QThread(this);
    c->connection->moveToThread(c->thread);
    c->thread->start();

    c->connection->initConnection();
    if (!connectedSpy.wait(500)) {
        return false;
    }

    c->queue = new EventQueue(this);
    c->queue->setup(c->connection);

    Registry registry;
    QSignalSpy interfacesAnnouncedSpy(&registry, &Registry::interfacesAnnounced);
    if (!interfacesAnnouncedSpy.isValid()) {
        return false;
    }
    registry.setEventQueue(c->queue);
    registry.create(c->connection);
    if (!registry.isValid()) {
        return false;
    }
    registry.setup();
    if (!interfacesAnnouncedSpy.wait(500)) {
        return false;
    }

    c->compositor = registry.createCompositor(registry.interface(Registry::Interface::Compositor).name,
                                              registry.interface(Registry::Interface::Compositor).version,
                                              this);
    if (!c->compositor->isValid()) {
        return false;
    }
    c->ddm = registry.createPrimarySelectionDeviceManagerV1(registry.interface(Registry::Interface::PrimarySelectionDeviceManagerV1).name,
                                              registry.interface(Registry::Interface::PrimarySelectionDeviceManagerV1).version,
                                              this);
    if (!c->ddm->isValid()) {
        return false;
    }
    c->seat = registry.createSeat(registry.interface(Registry::Interface::Seat).name,
                                  registry.interface(Registry::Interface::Seat).version,
                                  this);
    if (!c->seat->isValid()) {
        return false;
    }
    QSignalSpy keyboardSpy(c->seat, &Seat::hasKeyboardChanged);
    if (!keyboardSpy.isValid()) {
        return false;
    }
    if (!keyboardSpy.wait(500)) {
        return false;
    }
    if (!c->seat->hasKeyboard()) {
        return false;
    }
    c->keyboard = c->seat->createKeyboard(c->seat);
    if (!c->keyboard->isValid()) {
        return false;
    }
    c->dataDevice = c->ddm->getPrimarySelectDevice(c->seat, this);
    if (!c->dataDevice->isValid()) {
        return false;
    }

    return true;
}

void PrimarySelectionTest::cleanup()
{
    cleanupConnection(&m_client1);
    cleanupConnection(&m_client2);
#define CLEANUP(variable) \
        delete variable; \
        variable = nullptr;

    CLEANUP(m_ddmInterface)
    CLEANUP(m_seatInterface)
    CLEANUP(m_compositorInterface)
    CLEANUP(m_display)
#undef CLEANUP
}

void PrimarySelectionTest::cleanupConnection(Connection *c)
{
    delete c->dataDevice;
    c->dataDevice = nullptr;
    delete c->keyboard;
    c->keyboard = nullptr;
    delete c->ddm;
    c->ddm = nullptr;
    delete c->seat;
    c->seat = nullptr;
    delete c->compositor;
    c->compositor = nullptr;
    delete c->queue;
    c->queue = nullptr;
    if (c->connection) {
        c->connection->deleteLater();
        c->connection = nullptr;
    }
    if (c->thread) {
        c->thread->quit();
        c->thread->wait();
        delete c->thread;
        c->thread = nullptr;
    }
}

void PrimarySelectionTest::testClearOnEnter()
{
    // this test verifies that the selection is cleared prior to keyboard enter if there is no current selection
    QSignalSpy selectionClearedClient1Spy(m_client1.dataDevice, &PrimarySelectionDeviceV1::selectionCleared);
    QVERIFY(selectionClearedClient1Spy.isValid());
    QSignalSpy keyboardEnteredClient1Spy(m_client1.keyboard, &Keyboard::entered);
    QVERIFY(keyboardEnteredClient1Spy.isValid());

    // now create a Surface
    QSignalSpy surfaceCreatedSpy(m_compositorInterface, &CompositorInterface::surfaceCreated);
    QVERIFY(surfaceCreatedSpy.isValid());
    QScopedPointer<Surface> s1(m_client1.compositor->createSurface());
    QVERIFY(surfaceCreatedSpy.wait());
    auto serverSurface1 = surfaceCreatedSpy.first().first().value<SurfaceInterface*>();
    QVERIFY(serverSurface1);

    // pass this surface keyboard focus
    m_seatInterface->setFocusedKeyboardSurface(serverSurface1);
    // should get a clear
    QVERIFY(selectionClearedClient1Spy.wait());

    // let's set a selection
    QScopedPointer<PrimarySelectionSourceV1> dataSource(m_client1.ddm->createPrimarySelectSource());
    dataSource->offer(QStringLiteral("text/plain"));
    m_client1.dataDevice->setSelection(keyboardEnteredClient1Spy.first().first().value<quint32>(), dataSource.data());

    // now let's bring in client 2
    QSignalSpy selectionOfferedClient2Spy(m_client2.dataDevice, &PrimarySelectionDeviceV1::selectionOffered);
    QVERIFY(selectionOfferedClient2Spy.isValid());
    QSignalSpy selectionClearedClient2Spy(m_client2.dataDevice, &PrimarySelectionDeviceV1::selectionCleared);
    QVERIFY(selectionClearedClient2Spy.isValid());
    QSignalSpy keyboardEnteredClient2Spy(m_client2.keyboard, &Keyboard::entered);
    QVERIFY(keyboardEnteredClient2Spy.isValid());
    QScopedPointer<Surface> s2(m_client2.compositor->createSurface());
    QVERIFY(surfaceCreatedSpy.wait());
    auto serverSurface2 = surfaceCreatedSpy.last().first().value<SurfaceInterface*>();
    QVERIFY(serverSurface2);

    // entering that surface should give a selection offer
    m_seatInterface->setFocusedKeyboardSurface(serverSurface2);
    QVERIFY(selectionOfferedClient2Spy.wait());
    QVERIFY(selectionClearedClient2Spy.isEmpty());

    // set a data source but without offers
    QScopedPointer<PrimarySelectionSourceV1> dataSource2(m_client2.ddm->createPrimarySelectSource());
    m_client2.dataDevice->setSelection(keyboardEnteredClient2Spy.first().first().value<quint32>(), dataSource2.data());
    QVERIFY(selectionOfferedClient2Spy.wait());

}


QTEST_GUILESS_MAIN(PrimarySelectionTest)
#include "test_primaryselect.moc"
