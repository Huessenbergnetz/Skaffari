add_executable(skaffaricmd
    main.cpp
    consoleoutput.cpp
    consoleoutput.h
    setup.cpp
    setup.h
    database.cpp
    database.h
    imap.cpp
    imap.h
    webcyradmimporter.cpp
    webcyradmimporter.h
    configinput.cpp
    configinput.h
    ../common/password.cpp
    ../common/password.h
    ../common/global.h
    tester.cpp
    tester.h
    accountstatusupdater.cpp
    accountstatusupdater.h
    configfile.cpp
    configfile.h
    configchecker.cpp
    configchecker.h
    setupimporter.cpp
    setupimporter.h
)

target_compile_features(skaffaricmd
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

target_compile_options(skaffaricmd
    PRIVATE
        -Wall
        -Wcast-align
        -Wno-uninitialized
        -Wempty-body
        -Wformat-security
        -Wformat
        -Winit-self
)

target_compile_definitions(skaffaricmd
    PRIVATE
        QT_NO_KEYWORDS
        QT_NO_CAST_TO_ASCII
        QT_NO_CAST_FROM_ASCII
        QT_STRICT_ITERATORS
        QT_NO_URL_CAST_FROM_STRING
        QT_NO_CAST_FROM_BYTEARRAY
        QT_USE_QSTRINGBUILDER
        QT_SHA3_KECCAK_COMPAT
)

target_link_libraries(skaffaricmd
    PRIVATE
        Qt5::Core
        Qt5::Sql
        Qt5::Network
        Cutelyst::Authentication
        Cutelyst::Utils::Sql
        Cutelyst::Utils::Validator
        crypt
)

pkg_check_modules(SYSTEMD QUIET libsystemd)
if (SYSTEMD_FOUND)
    target_compile_definitions(skaffaricmd PRIVATE WITH_SYSTEMD)
endif(SYSTEMD_FOUND)

install(TARGETS skaffaricmd DESTINATION ${CMAKE_INSTALL_BINDIR})
