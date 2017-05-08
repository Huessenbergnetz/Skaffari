configure_file(skaffarictl.8.in ${CMAKE_BINARY_DIR}/doc/man/skaffarictl.8)
configure_file(skaffari.ini.5.in ${CMAKE_BINARY_DIR}/doc/man/skaffari.ini.5)

find_program(GZIP_BIN NAMES gzip)

execute_process(COMMAND ${GZIP_BIN} -9fk ${CMAKE_BINARY_DIR}/doc/man/skaffarictl.8 OUTPUT_QUIET)
execute_process(COMMAND ${GZIP_BIN} -9fk ${CMAKE_BINARY_DIR}/doc/man/skaffari.ini.5 OUTPUT_QUIET)

install(FILES ${CMAKE_BINARY_DIR}/doc/man/skaffarictl.8.gz DESTINATION ${CMAKE_INSTALL_MANDIR}/man8)
install(FILES ${CMAKE_BINARY_DIR}/doc/man/skaffari.ini.5.gz DESTINATION ${CMAKE_INSTALL_MANDIR}/man5)
