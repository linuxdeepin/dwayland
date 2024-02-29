add_executable(org-deepin-dwayland-testserver main.cpp testserver.cpp)
target_link_libraries(org-deepin-dwayland-testserver Qt${QT_MAJOR_VERSION}::Core D::WaylandServer)
install(TARGETS org-deepin-dwayland-testserver DESTINATION ${KDE_INSTALL_LIBEXECDIR} )
