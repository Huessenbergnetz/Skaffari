include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
) 

#file(GLOB_RECURSE skaffarictl_SRCS *.cpp *.h)
set(skaffarictl_SRCS
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
    tester.cpp
    tester.h
)

add_executable(skaffarictl ${skaffarictl_SRCS})

target_link_libraries(skaffarictl
    Qt5::Core
    Qt5::Sql
    Qt5::Network
    Cutelyst::Authentication
    Cutelyst::Utils::Sql
    crypt
)

install(TARGETS skaffarictl DESTINATION ${CMAKE_INSTALL_BINDIR})
