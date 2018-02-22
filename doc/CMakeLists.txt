set(docsources Doxyfile.in)

add_custom_target(skaffaridoxy SOURCES ${docsources})

configure_file(Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

add_subdirectory(man)
