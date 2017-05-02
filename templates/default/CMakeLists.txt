if (BUILD_TEMPLATES)

    find_program(NPM_BIN NAMES npm)

    message(STATUS "Installing required node.js dependencies for default template")

    execute_process(COMMAND ${NPM_BIN} update --dev WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_QUIET)

    message(STATUS "Installing required node.js dependencies for default template - done")

    message(STATUS "Building assets for default template")

    execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/node_modules/gulp/bin/gulp.js WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_QUIET)

    message(STATUS "Building assets for default template - done")

endif (BUILD_TEMPLATES)

install(DIRECTORY site static DESTINATION ${TEMPLATES_INSTALL_DIR}/default)
