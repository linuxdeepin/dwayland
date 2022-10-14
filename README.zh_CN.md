# KWayland

KWayland是一个qt风格的API，用于与通道-客户端和通道-服务器API交互。

## 的介绍
KWayland提供两个库:
- KWayland::客户端
- KWayland:服务器

顾名思义，它们分别为Wayland实现了一个客户端和一个服务器API
协议。这个API是Qt风格的，不需要与Qt开发人员进行交互
不舒服的低级c api。例如Wayland API的回调机制
由信号代替;数据类型被调整为Qt开发人员期望的类型，例如:
int的两个参数用QPoint或QSize表示。

## KWayland服务器
### Head-less API
这个服务器库可以用Qt实现一个Wayland服务器，API是无头的
这意味着它不执行任何输出，也不限制人们想要的方式
渲染。这使得基于OpenGL或OpenGL的现有渲染代码可以轻松集成
QPainter。构建在KWayland Server上的应用程序将图形与
以下技术:
- OpenGL over DRM/KMS
- OpenGL over X11
- OpenGL over Wayland
- OpenGL over Android's hwcomposer enabled through libhybris
- QPainter over DRM/KMs
- QPainter over fbdev
- QPainter over X11
- QPainter over Wayland
- QWidget
- QtQuick

尽管库不执行任何输出，但它使启用呈现变得非常容易。
[Buffer](@ref KWayland::Server::BufferInterface)的表示允许简单的转换
到(内存共享)QImage，如果缓冲区表示共享内存缓冲区。这个打印大师
可以用于在基于QPainter的API中渲染或生成OpenGL纹理。

### 轻松使用Wayland API

库隐藏了许多Wayland实现细节。对于所有的Wayland接口
双缓冲状态类总是只提供对提交状态的访问。悬而未决的
状态是一个内部细节。在提交挂起状态时，Qt信号是关于什么的
改变了。

缓冲区被引用计数，如果它不再被引用，则自动释放
客户端重用它。当一个表面不再引用缓冲区时，这将完全自动发生。
只要缓冲被附加到表面上，该表面就会引用它，API的用户就可以
访问缓冲区而不需要关心引用它。
KWayland的API是手工制作的，使用起来更容易。的表示法
[Surface](@ref KWayland::Server::SurfaceInterface)甚至结合了Surface的多个方面
如果在Wayland API中，它被添加到其他元素中。例如，一个Surface包含所有
[subsurface](@ref KWayland::Server::SubSurfaceInterface)附加到它而不是用户
必须监控SubSurface是为哪个表面创建的。
类似于[Seat](@ref KWayland::Server::SeatInterface)的表示结合了
的座位。API的用户只需要与Seat交互，不需要跟踪所有的
创建[keyboard](@ref KWayland::Server::KeyboardInterface)， [pointer](@ref KWayland::Server::PointerInterface)，等等。的
的座位轨迹的表示，这些键盘是生成的，并能够将事件转发到
适当的焦点表面，发送进入和离开通知时，需要的API的用户
去关心它。

### 处理输入事件
就像输出一样，服务器API不限制如何获取输入事件。这允许
与现有的输入处理程序集成，还允许在传递输入之前轻松地筛选输入
到服务器，然后从那里委托给客户端。这样就可以过滤掉例如全局触摸
手势或键盘快捷键，而不必在KWayland中实现处理程序。的SeatInterface
提供了一个非常容易使用的API来转发事件，可以很容易地与Qt自己的集成
输入事件系统，例如有一个从Qt::MouseButton到Linux输入代码的映射。
建立在KWayland Server之上的应用程序通过以下技术集成了输入事件:
- libinput
- X11
- 卫兰德
- Android的输入栈通过libhybris启用
- QInputEvent

### 带有子进程的私有IPC
KWayland Server非常适合拥有带有子进程的私有IPC。[Display](@ref KWayland::Server::Display)可以是
以不创建公共套接字而只允许通过套接字连接的方式进行设置
对。这允许创建一个套接字对，传递一个文件描述符到KWayland服务器和另一个
到分叉进程，例如通过WAYLAND_SOCKET环境变量。因此，一个专门的IPC
创建，甚至可以用于运行您自己的自定义协议。例如KDE Plasma使用
这样一个专用的父子Wayland服务器在它的屏幕锁架构中。
当然，私有套接字可以在任何时候添加到公共可用套接字之外。这是否可以用于识别特定的客户端，并仅限制对一些专用的接口的访问客户。

# # KWayland客户

围绕KWayland Client的想法是为Wayland客户端库提供一个drop-in API

同时提供了方便的qt风格API。它不打算被用作替代

QtWayland QPA插件，而是作为一种与Wayland交互的方式，以防需要使用Qt

一个不同的QPA插件或与QtWayland的组合，以允许更低级的交互

需要编写C代码。



# # #方便的API
KWayland Client中的便利API提供了一个封装Wayland对象的类。每个类都可以
铸造成包装威兰德型。该API将事件表示为信号，并提供简单的
方法调用请求。

可以通过[Registry](@ref KWayland::Client::Registry)创建表示全局Wayland资源的类。这门课放松
与Wayland注册表的交互，并在新的全局变量被宣布或获取时发出信号
移除。注册表有一个已知接口列表(例如常见的Wayland协议，如' wl_compositor ')
或' wl_shell ')，它们具有专用的宣布/删除信号和对象，可以由注册表分解
全局变量。

许多全局变量充当进一步资源的工厂。例如，Compositor有一个工厂方法
表面。所有的对象也可以用底层的方式创建，直接与Wayland API交互，
另外还要提供方便工厂的方法。这既允许简单的使用，也允许更低的级别
对Wayland API的控制。

### 与QtWayland QPA集成
如果QGuiApplication使用QtWayland QPA, KWayland允许与它集成。这是一个
不需要创建到Wayland服务器的新连接，但可以重用Qt使用的连接
是一种从Qt中获取Wayland对象的方法，各自的类提供了一个静态方法，通常称为
“fromApplication”。此外，API还允许从QWindow中获取Surface。

## 在您的应用程序中使用KWayland
### CMake
KWayland安装一个CMake配置文件，允许使用KWayland作为导入目标。有
一个库用于客户端，一个库用于服务器。
要查找包，请使用以下示例:
find_package (KF5Wayland配置)
set_package_properties(KF5Wayland属性类型可选)
add_feature_info("KF5Wayland" KF5Wayland_FOUND "为了不起的Wayland在Qt演示所需")
现在链接客户端库使用:
add_executable (exampleApp example.cpp)
target_link_libraries (exampleApp KF5:: WaylandClient)
要链接到使用的Server库:
add_executable (exampleServer exampleServer.cpp)
target_link_libraries (exampleServer KF5:: WaylandServer)

### QMake
KWayland为客户端和服务器库安装.pri文件，允许在QMake的基础上轻松使用
应用程序。
只使用:
QT + = KWaylandClient
分别为:
QT + = KWaylandServer
请确保您的项目配置了c++ 11支持:
配置+ = c++ 11
