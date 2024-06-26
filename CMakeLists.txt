cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
cmake_policy(SET CMP0048 NEW)

if (CMAKE_VERSION VERSION_GREATER 3.19.8)
    cmake_policy(SET CMP0115 NEW)
endif (CMAKE_VERSION VERSION_GREATER 3.19.8)

project(skaffari VERSION 1.0.0)

find_package(Qt5 5.6.0 REQUIRED COMPONENTS Core Network Sql Xml)
find_package(Cutelyst3Qt5 2.10.0 REQUIRED)
find_package(Cutelee6Qt5 5.2.0 REQUIRED)
find_package(PkgConfig REQUIRED)

# Auto generate moc files
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Skaffari default install prefix" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(SKAFFARI_APP_FILE "${CMAKE_INSTALL_FULL_LIBDIR}/cutelyst${Cutelyst3Qt5_VERSION_MAJOR}-qt5-apps/libskaffari.so")
set(APPDIR "${CMAKE_INSTALL_FULL_DATADIR}/skaffari")
set(CMAKE_INSTALL_LOCALEDIR "${APPDIR}/l10n" CACHE PATH "Directory to install core translations")
SET(LANGDIR "${APPDIR}/l10n" CACHE PATH "Directory to install core translations")
set(TEMPLATES_INSTALL_DIR "${APPDIR}/templates" CACHE PATH "Directory to install templates")
set(SKAFFARI_STATIC_INSTALL_DIR "${APPDIR}/static" CACHE PATH "Directory to install general static template files")
#set(CMAKE_INSTALL_SYSCONFDIR "/etc" CACHE PATH "Directory to install configuration files")
set(SKAFFARI_INI_FILE "${CMAKE_INSTALL_FULL_SYSCONFDIR}/skaffari.ini")
set(SQL_INSTALL_DIR "${APPDIR}/sql" CACHE PATH "Directory to install SQL files")
set(CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATADIR}/doc/skaffari" CACHE PATH "Directory to install documentation")
set(CMAKE_INSTALL_MANDIR "${CMAKE_INSTALL_DATADIR}/man" CACHE PATH "Directory to install man pages")
set(SKAFFARI_RUNTIME_DIR "/run/skaffari" CACHE PATH "Directory for runtime files (pid and socket)")
set(SKAFFARI_SOCKET_FILE "${SKAFFARI_RUNTIME_DIR}/skaffari.sock")
set(SKAFFARI_USER "skaffari" CACHE STRING "System user for Skaffari")
set(SKAFFARI_GROUP "skaffari" CACHE STRING "System group for Skaffari")
set(SKAFFARI_SOCKET_USER "${SKAFFARI_USER}" CACHE STRING "User for the Skaffari FastCGI socket file")
set(SKAFFARI_SOCKET_GROUP "www" CACHE STRING "Group for the Skaffari FastCGI Socket file")
set(SKAFFARI_CMD_PATH "${CMAKE_INSTALL_FULL_BINDIR}/skaffaricmd")
set(CUTELYST_WSGI_EXECUTABLE "${CMAKE_INSTALL_FULL_BINDIR}/cutelystd3-qt5" CACHE FILEPATH "Path to the cutelystd3 executable")
set(SYSTEMD_UNIT_DIR "${CMAKE_INSTALL_PREFIX}/lib/systemd/system" CACHE PATH "Systemd unit directory")
set(APACHE_VHOSTS_DIR "${CMAKE_INSTALL_FULL_SYSCONFDIR}/apache2/vhosts.d" CACHE PATH "Apache virtual hosts config directory")

option(INSTALL_SUPPLEMENTARY_FILES "Install systemd service and timer files as well as apache example config" OFF)

option(ENABLE_WKD "Enable support for Web Key Directory" OFF)
if (ENABLE_WKD)
    find_package(Qca-qt5 CONFIG REQUIRED)
endif (ENABLE_WKD)

option(BUILD_TESTS "Build the Skaffari tests" OFF)
if (BUILD_TESTS)
    enable_testing()
endif (BUILD_TESTS)

# set default configuration values
set(DEFVAL_ACC_PWMETHOD 1 CACHE INTERNAL "Default accounts password method")
set(DEFVAL_ACC_PWALGORITHM 0 CACHE INTERNAL "Default accounts password algorithm")
set(DEFVAL_ACC_PWROUNDS 32000 CACHE INTERNAL "Default accounts password iteration count")
set(DEFVAL_ACC_PWMINLENGTH 8 CACHE INTERNAL "Default accounts password minimum length")
set(DEFVAL_ACC_PWTHRESHOLD 30 CACHE INTERNAL "Default accounts password quality threshold")
set(DEFVAL_ADM_PWALGORITHM 4 CACHE INTERNAL "Default admin password algorithm")
set(DEFVAL_ADM_PWROUNDS 32000 CACHE INTERNAL "Default admin password iteration count")
set(DEFVAL_ADM_PWMINLENGTH 8 CACHE INTERNAL "Default admin password minimum length")
set(DEFVAL_ADM_PWTHRESHOLD 50 CACHE INTERNAL "Default admin password quality threshold")
set(DEFVAL_DEF_DOMAINQUOTA 0 CACHE INTERNAL "Default domain quota for new domains in KiB")
set(DEFVAL_DEF_LANGUAGE "en_US" CACHE INTERNAL "Default language")
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
set(DEFVAL_IMAP_AUTHMECH 0 CACHE INTERNAL "Default value for authmech")
set(DEFVAL_TMPL_ASYNCACCOUNTLIST false CACHE INTERNAL "Default value for async account list")

configure_file(common/config.h.in ${CMAKE_BINARY_DIR}/common/config.h)

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
add_subdirectory(supplementary)

if (BUILD_TESTS)
    add_subdirectory(tests)
endif (BUILD_TESTS)
