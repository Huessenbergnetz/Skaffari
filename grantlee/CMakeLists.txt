add_library(grantlee_skaffari MODULE
    filesizeformattag.cpp
    filesizeformattag.h
    timezoneconverttag.cpp
    timezoneconverttag.h
    admintypetag.cpp
    admintypetag.h
    urlencodefilter.cpp
    urlencodefilter.h
    acedecodefilter.cpp
    acedecodefilter.h
    stringlistsortfilter.cpp
    stringlistsortfilter.h
    splitfilter.cpp
    splitfilter.h
    stringformatfilter.cpp
    stringformatfilter.h
    skaffarigrantlee.cpp
    skaffarigrantlee.h
)

target_include_directories(grantlee_skaffari SYSTEM PRIVATE ${Cutelyst2Qt5_INCLUDE_DIR})

target_compile_features(grantlee_skaffari
    PRIVATE
        cxx_auto_type
        cxx_defaulted_move_initializers
        cxx_generalized_initializers
        cxx_lambdas
        cxx_long_long_type
        cxx_nonstatic_member_init
        cxx_nullptr
        cxx_override
        cxx_range_for
        cxx_right_angle_brackets
        cxx_strong_enums
        cxx_unicode_literals
        cxx_uniform_initialization
)

target_compile_options(grantlee_skaffari
    PRIVATE
        -Wall
        -Wcast-align
        -Wno-uninitialized
        -Wempty-body
        -Wformat-security
        -Wformat
        -Winit-self
)

target_link_libraries(grantlee_skaffari
    PRIVATE
        Grantlee5::Templates
        Cutelyst::Core
        Cutelyst::Session
        skaffari
)

set_property(TARGET grantlee_skaffari PROPERTY EXPORT_NAME skaffari)
grantlee_adjust_plugin_name(grantlee_skaffari)

install(TARGETS grantlee_skaffari EXPORT grantlee_targets
        LIBRARY DESTINATION ${GRANTLEE_PLUGIN_INSTALL_DIR} COMPONENT Templates
)
