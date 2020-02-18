add_custom_target(skaffaridoxy SOURCES
    Doxyfile.in
    DoxygenLayout.xml
)

configure_file(Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

add_subdirectory(man)
