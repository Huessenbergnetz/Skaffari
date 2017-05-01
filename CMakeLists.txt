project(skaffari)

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

set(SKAFFARI_VERSION_MAJOR "0")
set(SKAFFARI_VERSION_MINOR "0")
set(SKAFFARI_VERSION_PATCH "1")
set(SKAFFARI_VERSION "${SKAFFARI_VERSION_MAJOR}.${SKAFFARI_VERSION_MINOR}.${SKAFFARI_VERSION_PATCH}")

find_package(Qt5 5.6.0 COMPONENTS Core Network Sql)
find_package(CutelystQt5 1.5.0 REQUIRED)
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
set(APPDIR "${CMAKE_INSTALL_DATADIR}/skaffari")
set(CMAKE_INSTALL_LOCALEDIR "${APPDIR}/l10n" CACHE PATH "Directory to install translations")
set(TEMPLATES_INSTALL_DIR "${APPDIR}/templates" CACHE PATH "Directory to install templates")
set(CMAKE_INSTALL_SYSCONFDIR "/etc" CACHE PATH "Directory to install configuration files")
set(SQL_INSTALL_DIR "${APPDIR}/sql" CACHE PATH "Directory to install SQL files")
set(CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATADIR}/doc/skaffari" CACHE PATH "Directory to install documentation")

# Enable C++11 features
add_definitions(-std=c++11)

#add_definitions("-DSKAFFARI_VERSION=\"${SKAFFARI_VERSION}\"")
#add_definitions("-DSKAFFARI_L10NDIR=\"${CMAKE_INSTALL_LOCALEDIR}\"")
#add_definitions("-DSKAFFARI_TMPLDIR=\"${TEMPLATES_INSTALL_DIR}\"")
#add_definitions("-DSKAFFARI_CONFDIR=\"${CMAKE_INSTALL_SYSCONFDIR}\"")
#add_definitions("-DSKAFFARI_SQLDIR=\"${SQL_INSTALL_DIR}\"")
add_definitions("-Werror -Wall -Wcast-align -Wno-uninitialized -Wempty-body -Wformat-security -Wformat -Winit-self")

add_definitions(
    -DQT_NO_KEYWORDS
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_ASCII
    -DQT_STRICT_ITERATORS
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_USE_QSTRINGBUILDER
)

configure_file(common/config.h.in ${CMAKE_BINARY_DIR}/common/config.h)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CutelystQt5_INCLUDE_DIR}
)

add_subdirectory(src)
add_subdirectory(sql)
add_subdirectory(ctl)

