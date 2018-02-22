add_custom_target(skaffarimanpages SOURCES
    skaffaricmd.8.in
    skaffari.ini.5.in
    skaffari.8.in
)

configure_file(skaffaricmd.8.in ${CMAKE_BINARY_DIR}/doc/man/skaffaricmd.8)
configure_file(skaffari.ini.5.in ${CMAKE_BINARY_DIR}/doc/man/skaffari.ini.5)
configure_file(skaffari.8.in ${CMAKE_BINARY_DIR}/doc/man/skaffari.8)

install(FILES ${CMAKE_BINARY_DIR}/doc/man/skaffaricmd.8 DESTINATION ${CMAKE_INSTALL_MANDIR}/man8)
install(FILES ${CMAKE_BINARY_DIR}/doc/man/skaffari.ini.5 DESTINATION ${CMAKE_INSTALL_MANDIR}/man5)
install(FILES ${CMAKE_BINARY_DIR}/doc/man/skaffari.8 DESTINATION ${CMAKE_INSTALL_MANDIR}/man8)
