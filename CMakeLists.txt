project(skaffari)

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

set(SKAFFARI_VERSION_MAJOR "1")
set(SKAFFARI_VERSION_MINOR "0")
set(SKAFFARI_VERSION_PATCH "0")
set(SKAFFARI_VERSION "${SKAFFARI_VERSION_MAJOR}.${SKAFFARI_VERSION_MINOR}.${SKAFFARI_VERSION_PATCH}")

find_package(Qt5 5.6.0 COMPONENTS Core Network Sql)
find_package(Cutelyst2Qt5 2.0.0 REQUIRED)
find_package(Grantlee5 REQUIRED)
find_package(PkgConfig REQUIRED)

# Auto generate moc files
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Skaffari default install prefix" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(CMAKE_INSTALL_BINDIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Directory to install skaffarictl")
set(CMAKE_INSTALL_DATADIR "${CMAKE_INSTALL_PREFIX}/share" CACHE PATH "Data install directory")
set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Library install directory")
set(APPDIR "${CMAKE_INSTALL_DATADIR}/skaffari")
set(CMAKE_INSTALL_LOCALEDIR "${APPDIR}/l10n" CACHE PATH "Directory to install core translations")
set(TEMPLATES_INSTALL_DIR "${APPDIR}/templates" CACHE PATH "Directory to install templates")
set(CMAKE_INSTALL_SYSCONFDIR "/etc/skaffari" CACHE PATH "Directory to install configuration files")
set(SQL_INSTALL_DIR "${APPDIR}/sql" CACHE PATH "Directory to install SQL files")
set(CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATADIR}/doc/skaffari" CACHE PATH "Directory to install documentation")
set(CMAKE_INSTALL_MANDIR "${CMAKE_INSTALL_DATADIR}/man" CACHE PATH "Directory to install man pages")
set(BUILD_TEMPLATES OFF CACHE BOOL "If enabled, template assests will be build")

set(Grantlee5_MAJOR_MINOR_VERSION_STRING "${Grantlee5_VERSION_MAJOR}.${Grantlee5_VERSION_MINOR}" )
set(GRANTLEE_PLUGIN_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/grantlee/${Grantlee5_MAJOR_MINOR_VERSION_STRING} )

# Enable C++11 features
add_definitions(-std=c++11)

add_definitions("-Wall -Wcast-align -Wno-uninitialized -Wempty-body -Wformat-security -Wformat -Winit-self")

add_definitions(
    -DQT_NO_KEYWORDS
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_ASCII
    -DQT_STRICT_ITERATORS
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_USE_QSTRINGBUILDER
)

# set default configuration values
set(DEFVAL_ACC_PWMETHOD 1 CACHE INTERNAL "Default accounts password method")
set(DEFVAL_ACC_PWALGORITHM 0 CACHE INTERNAL "Default accounts password algorithm")
set(DEFVAL_ACC_PWROUNDS 32000 CACHE INTERNAL "Default accounts password iteration count")
set(DEFVAL_ACC_PWMINLENGTH 8 CACHE INTERNAL "Default accounts password minimum length")
set(DEFVAL_ADM_PWALGORITHM 4 CACHE INTERNAL "Default admin password algorithm")
set(DEFVAL_ADM_PWROUNDS 32000 CACHE INTERNAL "Default admin password iteration count")
set(DEFVAL_ADM_PWMINLENGTH 8 CACHE INTERNAL "Default admin password minimum length")
set(DEFVAL_DEF_DOMAINQUOTA 0 CACHE INTERNAL "Default domain quota for new domains in KiB")
set(DEFVAL_DEF_LANGUAGE "en" CACHE INTERNAL "Default language")
set(DEFVAL_DEF_MAXACCOUNTS 1000 CACHE INTERNAL "Default maximum accounts for new domains")
set(DEFVAL_DEF_QUOTA 102400 CACHE INTERNAL "Default quota for new accounts in KiB")
set(DEFVAL_DEF_TIMEZONE "UTC" CACHE INTERNAL "Default timezone")
set(DEFVAL_DEF_WARNLEVEL 90 CACHE INTERNAL "Default warn level for domain quota in percent")
set(DEFVAL_DEF_MAXDISPLAY 25 CACHE INTERNAL "Default value for displaying list entries")
set(DEFVAL_IMAP_PROTOCOL 2 CACHE INTERNAL "Default protocol for the IMAP connection")
set(DEFVAL_IMAP_ENCRYPTION 1 CACHE INTERNAL "Encryption layer for the IMAP connection")
set(DEFVAL_IMAP_CREATEMAILBOX 3 CACHE INTERNAL "Strategy to create new mailboxes on the IMAP server")
set(DEFVAL_IMAP_UNIXHIERARCHYSEP false CACHE INTERNAL "Default value for unix hierarchy separator")
set(DEFVAL_IMAP_DOMAINASPREFIX false CACHE INTERNAL "Default value for domain as prefix")
set(DEFVAL_IMAP_FQUN false CACHE INTERNAL "Default value for fqun")
set(DEFVAL_TMPL_ASYNCACCOUNTLIST false CACHE INTERNAL "Default value for async account list")


configure_file(common/config.h.in ${CMAKE_BINARY_DIR}/common/config.h)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Cutelyst2Qt5_INCLUDE_DIR}
)

find_program(LRELEASE_CMD_PATH NAMES lrelease-qt5 lrelease)
set(LRELEASE_CMD ${LRELEASE_CMD_PATH})
if(LRELEASE_CMD)
    message(STATUS "Found lrelease at ${LRELEASE_CMD}")
    message(STATUS "Translations are enabled")

    set(LANGS de)

else(LRELEASE_CMD)
    message(WARNING "lrelease executable cannot be found")
    message(WARNING "Translations are disabled")
endif(LRELEASE_CMD)

install(FILES contribute.json DESTINATION ${APPDIR}/static)

add_subdirectory(src)
add_subdirectory(sql)
add_subdirectory(cmd)
add_subdirectory(templates)
add_subdirectory(doc)
add_subdirectory(l10n)
add_subdirectory(grantlee)
