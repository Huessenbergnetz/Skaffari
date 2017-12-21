add_subdirectory(l10n)

install(DIRECTORY site static DESTINATION ${TEMPLATES_INSTALL_DIR}/default)

install(FILES metadata.json DESTINATION ${TEMPLATES_INSTALL_DIR}/default)
