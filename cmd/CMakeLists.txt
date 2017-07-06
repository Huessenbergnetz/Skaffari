include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
) 

set(skaffaricmd_SRCS
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
)

add_executable(skaffaricmd ${skaffaricmd_SRCS})

target_link_libraries(skaffaricmd
    Qt5::Core
    Qt5::Sql
    Qt5::Network
    Cutelyst::Authentication
    Cutelyst::Utils::Sql
    crypt
)

install(TARGETS skaffaricmd DESTINATION ${CMAKE_INSTALL_BINDIR})
