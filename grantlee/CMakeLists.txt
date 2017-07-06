add_library(grantlee_skaffari MODULE
    filesizeformattag.cpp
    filesizeformattag.h
    skaffarigrantlee.cpp
)

set_property(TARGET grantlee_skaffari PROPERTY EXPORT_NAME skaffari)
target_link_libraries(grantlee_skaffari
    PRIVATE Grantlee5::Templates
    PRIVATE Cutelyst::Core
)
grantlee_adjust_plugin_name(grantlee_skaffari)

install(TARGETS grantlee_skaffari EXPORT grantlee_targets
        LIBRARY DESTINATION ${GRANTLEE_PLUGIN_INSTALL_DIR} COMPONENT Templates
)
