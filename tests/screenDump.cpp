/********************************************************************
Copyright 2015  Martin Gräßlin <mgraesslin@kde.org>

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
#include "../src/client/event_queue.h"
#include "../src/client/registry.h"
#include "../src/client/output.h"
#include "../src/client/remote_access.h"
// Qt
#include <QGuiApplication>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QTime>
#include <QImage>
#include <QStringList>
// system
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

using namespace KWayland::Client;

class KwaylandTest : public QObject
{
    Q_OBJECT
public:
    explicit KwaylandTest(QObject *parent = nullptr);
    virtual ~KwaylandTest();

    void processBuffer(const Output *m_output, const RemoteBuffer* m_remoteBuffer);

    void init();

private:
    void setupRegistry(Registry *registry);
    QThread *m_connectionThread;
    ConnectionThread *m_connectionThreadObject;
    EventQueue *m_eventQueue = nullptr;
    Compositor *m_compositor = nullptr;
    RemoteAccessManager *m_remoteAccessManager = nullptr;
    QList<Output*> m_outputList;
    Output *m_bufferReadyOutput = nullptr;
    Output *m_output0 = nullptr;
    Output *m_output1 = nullptr;
};

KwaylandTest::KwaylandTest(QObject *parent)
    : QObject(parent)
    , m_connectionThread(new QThread(this))
    , m_connectionThreadObject(new ConnectionThread())
{
}

KwaylandTest::~KwaylandTest()
{
    m_connectionThread->quit();
    m_connectionThread->wait();
    m_connectionThreadObject->deleteLater();
}

void KwaylandTest::init()
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

void KwaylandTest::processBuffer(const Output *m_output, const RemoteBuffer* m_remoteBuffer)
{
    if (!m_remoteBuffer) {
        qDebug() <<QDateTime::currentDateTime().toString(Qt::ISODate)<<Q_FUNC_INFO<< "m_remoteBuffer is nullptr";
        return;
    }

    auto dma_fd = m_remoteBuffer->fd();
    quint32 width = m_remoteBuffer->width();
    quint32 height = m_remoteBuffer->height();
    quint32 stride = m_remoteBuffer->stride();

    unsigned char *mapData = static_cast<unsigned char *>(mmap(nullptr, stride * height, PROT_READ, MAP_SHARED, dma_fd, 0));
    if (MAP_FAILED == mapData)
    {
        qDebug() << "dma fd " << dma_fd <<" mmap failed - ";
    } else {
        qDebug() << "success dma fd " << dma_fd << "width"<<width<<"height"<<height;
        QImage destImage = QImage(mapData, width, height, QImage::Format_RGB32);
        static int count = 0;
        QString filename = QString::fromLatin1("/tmp/output%1.png").arg(count++);
        destImage.save(filename);
    }
    munmap(mapData, stride * height);
    close(dma_fd);
}

void KwaylandTest::setupRegistry(Registry *registry)
{
    connect(registry, &Registry::compositorAnnounced, this,
        [this, registry](quint32 name, quint32 version) {
            m_compositor = registry->createCompositor(name, version, this);
        }
    );

    connect(registry, &Registry::outputAnnounced, this,
        [this, registry] (quint32 name, quint32 version) {
            auto output = registry->createOutput(name, version, this);
            if (output) {
                qDebug() << QDateTime::currentDateTime().toString(QLatin1String("hh:mm:ss.zzz ")) << "Get output"<<name;
                m_outputList << output;
                if (!m_output0) {
                    m_output0 = output;
                }
                if (!m_output1 && (output != m_output0)) {
                    m_output1 = output;
                }
            }
        }
    );

    connect(registry, &Registry::remoteAccessManagerAnnounced, this,
        [this, registry](quint32 name, quint32 version) {
            m_remoteAccessManager = registry->createRemoteAccessManager(name, version, this);
            connect(m_remoteAccessManager, &RemoteAccessManager::bufferReady, this,
                [this] (const void *output, const RemoteBuffer * m_remoteBuffer)
                {
                    m_bufferReadyOutput = Output::get(reinterpret_cast<wl_output*>(const_cast<void*>(output)));
                    qDebug() << "RemoteAccessManager::bufferReady output"<< m_bufferReadyOutput->manufacturer()<<m_bufferReadyOutput->geometry();
                    connect(m_remoteBuffer, &RemoteBuffer::parametersObtained, this,
                        [this, m_remoteBuffer] {
                            processBuffer(m_bufferReadyOutput, m_remoteBuffer);
                        }
                    );
                }
            );
        }
    );

    connect(registry, &Registry::interfacesAnnounced, this,
        [this] {
            Q_ASSERT(m_compositor);
        }
    );

    registry->setEventQueue(m_eventQueue);
    registry->create(m_connectionThreadObject);
    registry->setup();
}

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    KwaylandTest client;
    client.init();

    return app.exec();
}

#include "screenDump.moc"
