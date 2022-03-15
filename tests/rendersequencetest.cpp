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
#include "../src/client/connection_thread.h"
#include "../src/client/clientmanagement.h"
#include "../src/client/event_queue.h"
#include "../src/client/registry.h"
#include "../src/client/remote_access.h"
#include "../src/client/output.h"

// Qt
#include <QGuiApplication>
#include <QDebug>
#include <QThread>
#include <QTime>
// system
#include <unistd.h>

using namespace KWayland::Client;

class RenderSequenceTest : public QObject
{
    Q_OBJECT
public:
    explicit RenderSequenceTest(QObject *parent = nullptr);
    virtual ~RenderSequenceTest();
    void init();

private:
    void setupRegistry(Registry *registry);
    QThread *m_connectionThread;
    ConnectionThread *m_connectionThreadObject;
    EventQueue *m_eventQueue = nullptr;
    RemoteAccessManager* m_remoteAccessManager = nullptr;
    QVector<ClientManagement::WindowState> m_windowStates;
};

RenderSequenceTest::RenderSequenceTest(QObject *parent)
    : QObject(parent)
    , m_connectionThread(new QThread(this))
    , m_connectionThreadObject(new ConnectionThread())
{
}

RenderSequenceTest::~RenderSequenceTest()
{
    m_connectionThread->quit();
    m_connectionThread->wait();
    m_connectionThreadObject->deleteLater();
}

void RenderSequenceTest::init()
{
    connect(m_connectionThreadObject, &ConnectionThread::connected, this,
        [this] {
            m_eventQueue = new EventQueue(this);
            m_eventQueue->setup(m_connectionThreadObject);

            Registry *registry = new Registry(this);
            setupRegistry(registry);
        },
        Qt::QueuedConnection
    );
    m_connectionThreadObject->moveToThread(m_connectionThread);
    m_connectionThread->start();

    m_connectionThreadObject->initConnection();
}

void RenderSequenceTest::setupRegistry(Registry *registry)
{
    connect(registry, &Registry::remoteAccessManagerAnnounced, this,
        [this, registry] (quint32 name, quint32 version) {
            m_remoteAccessManager = registry->createRemoteAccessManager(name , version, this);
            qDebug() << QDateTime::currentDateTime().toString(QLatin1String("hh:mm:ss.zzz ")) << "createRemoteAccessManager";
            m_remoteAccessManager->setEventQueue(m_eventQueue);
            m_remoteAccessManager->getRendersequence();
            connect(m_remoteAccessManager, &RemoteAccessManager::renderSequence, this, [](int number) {
                qDebug() << "============ get_rendersequence: " << number;
            });
        }
    );

    registry->setEventQueue(m_eventQueue);
    registry->create(m_connectionThreadObject);
    registry->setup();
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    RenderSequenceTest client;
    client.init();

    return app.exec();
}
#include "rendersequencetest.moc"

