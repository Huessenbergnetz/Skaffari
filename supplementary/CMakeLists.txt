add_custom_target(skaffarisupplementary SOURCES
    skaffari.ini.in
    skaffari.service.in
    skaffari-update-account-status.service.in
    skaffari-update-account-status.timer
    skaffari.conf.template.in
)

configure_file(skaffari.service.in ${CMAKE_BINARY_DIR}/supplementary/skaffari.service)
configure_file(skaffari-update-account-status.service.in ${CMAKE_BINARY_DIR}/supplementary/skaffari-update-account-status.service)
configure_file(skaffari.conf.template.in ${CMAKE_BINARY_DIR}/supplementary/skaffari.conf.template)
configure_file(skaffari.ini.in ${CMAKE_BINARY_DIR}/supplementary/skaffari.ini)

install(FILES ${CMAKE_BINARY_DIR}/supplementary/skaffari.ini DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR})

if(INSTALL_SUPPLEMENTARY_FILES)
    install(FILES
        ${CMAKE_BINARY_DIR}/supplementary/skaffari.service
        ${CMAKE_BINARY_DIR}/supplementary/skaffari-update-account-status.service
        skaffari-update-account-status.timer
        DESTINATION ${SYSTEMD_UNIT_DIR}
    )

    install(FILES ${CMAKE_BINARY_DIR}/supplementary/skaffari.conf.template DESTINATION ${APACHE_VHOSTS_DIR})
endif(INSTALL_SUPPLEMENTARY_FILES)
