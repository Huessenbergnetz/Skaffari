set (skaffari_SRCS
    imap/skaffariimap.cpp
    imap/skaffariimap.h
    imap/skaffariimaperror.cpp
    imap/skaffariimaperror.h
    objects/account.cpp
    objects/account.h
    objects/account_p.h
    objects/adminaccount.cpp
    objects/adminaccount.h
    objects/adminaccount_p.h
    objects/domain.cpp
    objects/domain.h
    objects/domain_p.h
    objects/simpleadmin.cpp
    objects/simpleadmin.h
    objects/simpledomain.cpp
    objects/simpledomain.h
    objects/skaffarierror.cpp
    objects/skaffarierror.h
    objects/folder.cpp
    objects/folder.h
    objects/helpentry.cpp
    objects/helpentry.h
    objects/simpleaccount.cpp
    objects/simpleaccount.h
    objects/emailaddress.cpp
    objects/emailaddress.h
    utils/language.cpp
    utils/language.h
    utils/utils.cpp
    utils/utils.h
    utils/skaffariconfig.cpp
    utils/skaffariconfig.h
    utils/qtimezonevariant_p.h
    accounteditor.cpp
    accounteditor.h
    admineditor.cpp
    admineditor.h
    authstoresql.cpp
    authstoresql.h
    domaineditor.cpp
    domaineditor.h
    settingseditor.cpp
    settingseditor.h
    login.cpp
    login.h
    logout.cpp
    logout.h
    myaccount.cpp
    myaccount.h
    root.cpp
    root.h
    skaffari.cpp
    skaffari.h
    ../common/password.cpp
    ../common/password.h
    ../common/global.h
    validators/skvalidatoruniquedb.cpp
    validators/skvalidatoruniquedb.h
    validators/skvalidatoraccountexists.cpp
    validators/skvalidatoraccountexists.h
    validators/skvalidatordomainexists.cpp
    validators/skvalidatordomainexists.h
)

if (ENABLE_WKD)
endif (ENABLE_WKD)

add_library(skaffari SHARED ${skaffari_SRCS})

pkg_check_modules(ICU REQUIRED icu-uc)

target_include_directories(skaffari
    SYSTEM PRIVATE
        ${Cutelyst2Qt5_INCLUDE_DIR}
        ${ICU_INCLUDE_DIRS}
)

target_compile_features(skaffari
    PRIVATE
        cxx_auto_type
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

target_compile_definitions(skaffari
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

target_compile_options(skaffari
    PRIVATE
        -Wall
        -Wcast-align
        -Wno-uninitialized
        -Wempty-body
        -Wformat-security
        -Wformat
        -Winit-self
)

# Link to Cutelyst
target_link_libraries(skaffari
    PRIVATE
        Qt5::Core
        Qt5::Network
        Qt5::Sql
        Cutelyst::Core
        Cutelyst::Session
        Cutelyst::Authentication
        Cutelyst::StatusMessage
        Cutelyst::View::Grantlee
        Cutelyst::StaticSimple
        Cutelyst::Utils::Validator
        Cutelyst::Utils::Sql
        Cutelyst::Utils::Pagination
        Cutelyst::Utils::LangSelect
        Cutelyst::Memcached
        Cutelyst::MemcachedSessionStore
        Cutelyst::CSRFProtection
        Grantlee5::Templates
        crypt
        ${ICU_LIBRARIES}
)

if (ENABLE_WKD)
    target_link_libraries(skaffari PRIVATE qca-qt5)
    target_compile_definitions(skaffari PRIVATE WITH_WKD)
endif (ENABLE_WKD)

get_target_property(CUTELYST_VALIDATOR_DEFINITIONS Cutelyst::Utils::Validator INTERFACE_COMPILE_DEFINITIONS)
string(FIND ${CUTELYST_VALIDATOR_DEFINITIONS} "CUTELYST_VALIDATOR_WITH_PWQUALITY" CUTELYST_VALIDATOR_CUTELYST_VALIDATOR_WITH_PWQUALITY)
if (CUTELYST_VALIDATOR_CUTELYST_VALIDATOR_WITH_PWQUALITY GREATER -1)
    message(STATUS "Cutelyst ValidatorPwQuality is available.")
    pkg_search_module(PWQUALITY REQUIRED pwquality>=1.2.2)
    target_compile_definitions(skaffari PRIVATE LIBPWQUALITY_VERSION="${PWQUALITY_VERSION}")
    target_link_libraries(skaffari PRIVATE ${PWQUALITY_LIBRARIES})
    target_include_directories(skaffari PRIVATE ${PWQUALITY_INCLUDE_DIRS})
endif (CUTELYST_VALIDATOR_CUTELYST_VALIDATOR_WITH_PWQUALITY GREATER -1)

pkg_check_modules(SYSTEMD QUIET libsystemd)
if (SYSTEMD_FOUND)
    message(STATUS "Found systemd version ${SYSTEMD_VERSION}. Enabling systemd journal support.")
    target_compile_definitions(skaffari PRIVATE WITH_SYSTEMD)
    target_link_libraries(skaffari PRIVATE ${SYSTEMD_LIBRARIES})
    target_include_directories(skaffari PRIVATE ${SYSTEMD_INCLUDE_DIRS})
else(SYSTEMD_FOUND)
    message(STATUS "Can not find systemd. Disabling systemd journal support.")
endif(SYSTEMD_FOUND)

install(TARGETS skaffari DESTINATION ${CMAKE_INSTALL_LIBDIR}/cutelyst${Cutelyst2Qt5_VERSION_MAJOR}-apps)
