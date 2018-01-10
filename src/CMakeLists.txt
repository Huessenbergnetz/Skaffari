set(skaffari_SRCS
    imap/skaffariimap.cpp
    imap/skaffariimap.h
    imap/skaffariimaperror.cpp
    imap/skaffariimaperror.h
    imap/skaffariimaperror_p.h
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
    objects/simpleadmin_p.h
    objects/simpledomain.cpp
    objects/simpledomain.h
    objects/simpledomain_p.h
    objects/skaffarierror.cpp
    objects/skaffarierror.h
    objects/skaffarierror_p.h
    objects/folder.cpp
    objects/folder.h
    objects/folder_p.h
    objects/helpentry.cpp
    objects/helpentry.h
    objects/helpentry_p.h
    objects/simpleaccount.cpp
    objects/simpleaccount.h
    objects/simpleaccount_p.h
    utils/language.cpp
    utils/language.h
    utils/language_p.h
    utils/utils.cpp
    utils/utils.h
    utils/skaffariconfig.cpp
    utils/skaffariconfig.h
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
    validators/skvalidatorfilesize.cpp
    validators/skvalidatorfilesize.h
)

# Create the application
add_library(skaffari SHARED ${skaffari_SRCS})

pkg_search_module(ICU REQUIRED icu-io)

# Link to Cutelyst
target_link_libraries(skaffari
    Cutelyst::Core
    Cutelyst::View::Grantlee
    Cutelyst::StaticSimple
    Cutelyst::Utils::Validator
    Cutelyst::Utils::Sql
    Cutelyst::Session
    Cutelyst::Authentication
    Cutelyst::StatusMessage
    Cutelyst::Utils::Pagination
    Cutelyst::Memcached
    Cutelyst::MemcachedSessionStore
    Cutelyst::CSRFProtection
    Grantlee5::Templates
    Qt5::Core
    Qt5::Network
    Qt5::Sql
    crypt
    ${ICU_LIBRARIES}
)

find_package(PkgConfig QUIET)
if (PKG_CONFIG_FOUND)
    pkg_search_module(SYSTEMD QUIET systemd libsystemd)
    if (SYSTEMD_FOUND)
        message(STATUS "Found systemd version ${SYSTEMD_VERSION}. Enabling systemd journal support.")
        add_definitions("-DWITH_SYSTEMD")
        target_link_libraries(skaffari systemd)
    else(SYSTEMD_FOUND)
        message(STATUS "Can not find systemd. Disabling systemd journal support.")
    endif(SYSTEMD_FOUND)
else(PKG_CONFIG_FOUND)
    message(STATUS "Can not find pkg-config. Disabling systemd journal support.")
endif(PKG_CONFIG_FOUND)

install(TARGETS skaffari DESTINATION ${CMAKE_INSTALL_LIBDIR}/cutelyst-apps)
