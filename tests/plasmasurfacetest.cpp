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
#include "../src/client/compositor.h"
#include "../src/client/connection_thread.h"
#include "../src/client/event_queue.h"
#include "../src/client/registry.h"
#include "../src/client/shell.h"
#include "../src/client/shm_pool.h"
#include "../src/client/surface.h"
#include "../src/client/plasmashell.h"
#include "../src/client/server_decoration.h"
#include "../src/client/xdgdecoration.h"
#include "../src/client/ddeshell.h"
// Qt
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QImage>
#include <QThread>

#include <QDebug>

using namespace KWayland::Client;

class PlasmaSurfaceTest : public QObject
{
    Q_OBJECT
public:
    explicit PlasmaSurfaceTest(QObject *parent = nullptr);
    virtual ~PlasmaSurfaceTest();

    void init();

    void setRole(PlasmaShellSurface::Role role) {
        m_role = role;
    }
    void setSkipTaskbar(bool set) {
        m_skipTaskbar = set;
    }

    void setSkipSwitcher(bool set) {
        m_skipSwitcher = set;
    }

private:
    void setupRegistry(Registry *registry);
    void render();
    QThread *m_connectionThread;
    ConnectionThread *m_connectionThreadObject;
    EventQueue *m_eventQueue = nullptr;
    Compositor *m_compositor = nullptr;
    Shell *m_shell = nullptr;
    ShellSurface *m_shellSurface = nullptr;
    ShmPool *m_shm = nullptr;
    Surface *m_surface = nullptr;
    PlasmaShell *m_plasmaShell = nullptr;
    PlasmaShellSurface *m_plasmaShellSurface = nullptr;
    PlasmaShellSurface::Role m_role = PlasmaShellSurface::Role::Normal;

    DDEShell *m_ddeShell = nullptr;
    DDEShellSurface *m_ddeShellSurface = nullptr;

    bool m_skipTaskbar = false;
    bool m_skipSwitcher = false;

    KWayland::Client::ServerSideDecorationManager *m_decoration = nullptr;
    KWayland::Client::XdgDecorationManager *m_xdgDecoration = nullptr;
};

PlasmaSurfaceTest::PlasmaSurfaceTest(QObject *parent)
    : QObject(parent)
    , m_connectionThread(new QThread(this))
    , m_connectionThreadObject(new ConnectionThread())
{
}

PlasmaSurfaceTest::~PlasmaSurfaceTest()
{
    m_connectionThread->quit();
    m_connectionThread->wait();
    m_connectionThreadObject->deleteLater();
}

void PlasmaSurfaceTest::init()
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

void PlasmaSurfaceTest::setupRegistry(Registry *registry)
{
    connect(registry, &Registry::compositorAnnounced, this,
        [this, registry](quint32 name, quint32 version) {
            m_compositor = registry->createCompositor(name, version, this);
        }
    );
    connect(registry, &Registry::shellAnnounced, this,
        [this, registry](quint32 name, quint32 version) {
            m_shell = registry->createShell(name, version, this);
        }
    );
    connect(registry, &Registry::shmAnnounced, this,
        [this, registry](quint32 name, quint32 version) {
            m_shm = registry->createShmPool(name, version, this);
        }
    );
    connect(registry, &Registry::plasmaShellAnnounced, this,
        [this, registry](quint32 name, quint32 version) {
            m_plasmaShell = registry->createPlasmaShell(name, version, this);
            m_plasmaShell->setEventQueue(m_eventQueue);
        }
    );
    connect(registry, &Registry::ddeShellAnnounced, this,
        [this, registry](quint32 name, quint32 version) {
            m_ddeShell = registry->createDDEShell(name, version, this);
            m_ddeShell->setEventQueue(m_eventQueue);
        }
    );
    connect(registry, &Registry::serverSideDecorationManagerAnnounced, this,
        [this, registry](quint32 name, quint32 version) {
            m_decoration = registry->createServerSideDecorationManager(name, version, this);
            m_decoration->setEventQueue(m_eventQueue);
        }
    );
    connect(registry, &Registry::xdgDecorationAnnounced, this,
        [this, registry](quint32 name, quint32 version) {
            m_xdgDecoration = registry->createXdgDecorationManager(name, version, this);
            m_xdgDecoration->setEventQueue(m_eventQueue);
        }
    );
    connect(registry, &Registry::interfacesAnnounced, this,
        [this] {
            Q_ASSERT(m_compositor);
            Q_ASSERT(m_shell);
            Q_ASSERT(m_shm);
            Q_ASSERT(m_plasmaShell);
            Q_ASSERT(m_ddeShell);
            m_surface = m_compositor->createSurface(this);
            Q_ASSERT(m_surface);
            m_shellSurface = m_shell->createSurface(m_surface, this);
            Q_ASSERT(m_shellSurface);
            m_shellSurface->setToplevel();
            connect(m_shellSurface, &ShellSurface::sizeChanged, this, &PlasmaSurfaceTest::render);
            m_plasmaShellSurface = m_plasmaShell->createSurface(m_surface, this);
            m_plasmaShellSurface->setPosition(QPoint(300, 200));
            m_ddeShellSurface = m_ddeShell->createShellSurface(m_surface, this);
            connect(m_ddeShellSurface, &DDEShellSurface::geometryChanged, this,
                [this] (const QRect &geom) {
                        qDebug() << "get geom" << geom;
                }
            );
            connect(m_ddeShellSurface, &DDEShellSurface::activeChanged, this,
                [this] {
                    qDebug() << "Window active changed: " << m_ddeShellSurface->isActive();
                }
            );
            connect(m_ddeShellSurface, &DDEShellSurface::maximizedChanged, this,
                [this] {
                    qDebug() << "Window maximized changed: " << m_ddeShellSurface->isMaximized();
                }
            );
            connect(m_ddeShellSurface, &DDEShellSurface::maximizedChanged, this,
                [this] {
                    qDebug() << "Window minimized changed: " << m_ddeShellSurface->isMinimized();
                }
            );
            connect(m_ddeShellSurface, &DDEShellSurface::keepAboveChanged, this,
                [this] {
                    qDebug() << "Window keep above changed: " << m_ddeShellSurface->isKeepAbove();
                }
            );
            connect(m_ddeShellSurface, &DDEShellSurface::keepBelowChanged, this,
                [this] {
                    qDebug() << "Window keep below changed: " << m_ddeShellSurface->isKeepBelow();
                }
            );
            connect(m_ddeShellSurface, &DDEShellSurface::fullscreenChanged, this,
                [this] {
                    qDebug() << "Window full screen changed: " << m_ddeShellSurface->isFullscreen();
                }
            );
            connect(m_ddeShellSurface, &DDEShellSurface::closeableChanged, this,
                [this] {
                    qDebug() << "Window is closeable changed: " << m_ddeShellSurface->isCloseable();
                }
            );
            connect(m_ddeShellSurface, &DDEShellSurface::minimizeableChanged, this,
                [this] {
                    qDebug() << "Window is minimizeable changed: " << m_ddeShellSurface->isMinimizeable();
                }
            );
            connect(m_ddeShellSurface, &DDEShellSurface::maximizeableChanged, this,
                [this] {
                    qDebug() << "Window is maximizeable changed: " << m_ddeShellSurface->isMaximizeable();
                }
            );
            connect(m_ddeShellSurface, &DDEShellSurface::fullscreenableChanged, this,
                [this] {
                    qDebug() << "Window is fullscreenable changed: " << m_ddeShellSurface->isFullscreenable();
                }
            );
            Q_ASSERT(m_plasmaShellSurface);
            m_plasmaShellSurface->setSkipTaskbar(m_skipTaskbar);
            m_plasmaShellSurface->setSkipSwitcher(m_skipSwitcher);
            m_plasmaShellSurface->setRole(m_role);

            // //创建标题栏server decoration
            auto parentDeco = m_decoration->create(m_surface, this);
            //设置csd/ssd
            parentDeco->requestMode(KWayland::Client::ServerSideDecoration::Mode::Server);

            m_ddeShellSurface->requestKeepAbove(true);

            //创建标题栏server decoration
            // auto xdgDecoration = m_xdgDecoration->getToplevelDecoration(m_xdgshellSurface, this);
            // //设置csd/ssd
            // xdgDecoration->setMode(KWayland::Client::XdgDecoration::Mode::ServerSide);

            render();
        }
    );
    registry->setEventQueue(m_eventQueue);
    registry->create(m_connectionThreadObject);
    registry->setup();
}

void PlasmaSurfaceTest::render()
{
    const QSize &size = m_shellSurface->size().isValid() ? m_shellSurface->size() : QSize(300, 200);
    auto buffer = m_shm->getBuffer(size, size.width() * 4).toStrongRef();
    buffer->setUsed(true);
    QImage image(buffer->address(), size.width(), size.height(), QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(255, 255, 255, 128));

    m_surface->attachBuffer(*buffer);
    m_surface->damage(QRect(QPoint(0, 0), size));
    m_surface->commit(Surface::CommitFlag::None);
    buffer->setUsed(false);
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption notificationOption(QStringLiteral("notification"));
    parser.addOption(notificationOption);
    QCommandLineOption panelOption(QStringLiteral("panel"));
    parser.addOption(panelOption);
    QCommandLineOption desktopOption(QStringLiteral("desktop"));
    parser.addOption(desktopOption);
    QCommandLineOption osdOption(QStringLiteral("osd"));
    parser.addOption(osdOption);
    QCommandLineOption tooltipOption(QStringLiteral("tooltip"));
    parser.addOption(tooltipOption);
    QCommandLineOption overrideOption(QStringLiteral("override"));
    parser.addOption(overrideOption);
    QCommandLineOption skipTaskbarOption(QStringLiteral("skipTaskbar"));
    parser.addOption(skipTaskbarOption);
    parser.process(app);
    QCommandLineOption skipSwitcherOption(QStringLiteral("skipSwitcher"));
    parser.addOption(skipSwitcherOption);
    parser.process(app);

    PlasmaSurfaceTest client;

    if (parser.isSet(notificationOption)) {
        client.setRole(PlasmaShellSurface::Role::Notification);
    } else if (parser.isSet(panelOption)) {
        client.setRole(PlasmaShellSurface::Role::Panel);
    } else if (parser.isSet(desktopOption)) {
        client.setRole(PlasmaShellSurface::Role::Desktop);
    } else if (parser.isSet(osdOption)) {
        client.setRole(PlasmaShellSurface::Role::OnScreenDisplay);
    } else if (parser.isSet(tooltipOption)) {
        client.setRole(PlasmaShellSurface::Role::ToolTip);
    } else if (parser.isSet(overrideOption)) {
        client.setRole(PlasmaShellSurface::Role::Override);
    }
    client.setSkipTaskbar(parser.isSet(skipTaskbarOption));
    client.setSkipSwitcher(parser.isSet(skipSwitcherOption));

    client.init();

    return app.exec();
}

#include "plasmasurfacetest.moc"
