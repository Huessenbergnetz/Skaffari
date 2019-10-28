project(skaffari_tests)

find_package(Qt5Test 5.6.0 REQUIRED)

add_library(skapp_test STATIC skapptestobject.cpp skapptestobject.h)

target_compile_features(skapp_test
    PRIVATE
        cxx_auto_type
    PUBLIC
        cxx_nullptr
        cxx_override
)

target_link_libraries(skapp_test
    PUBLIC
        Qt5::Test
        Qt5::Sql
        Qt5::Network
        skaffari
)

add_library(skweb_test STATIC skwebtest.cpp skwebtest.h)

target_compile_features(skweb_test
    PRIVATE
        cxx_auto_type
    PUBLIC
        cxx_nullptr
        cxx_override
)

target_link_libraries(skweb_test
    PUBLIC
        skapp_test
)

set(SKAFFARI_CMD_EXE "${CMAKE_BINARY_DIR}/cmd/skaffaricmd")

target_compile_definitions(skapp_test
    PRIVATE
        SKAFFARI_CMD="${SKAFFARI_CMD_EXE}"
)

function(skaffari_test _testname _link1 _link2 _link3)
    add_executable(${_testname}_exec ${_testname}.cpp)
    add_test(NAME ${_testname} COMMAND ${_testname}_exec)
    target_compile_features(${_testname}_exec PRIVATE cxx_nullptr)
    target_link_libraries(${_testname}_exec Qt5::Test ${_link1} ${_link2} ${_link3} skaffari)
#    target_include_directories(${_testname}_exec SYSTEM PRIVATE ${Cutelyst2Qt5_INCLUDE_DIR})
endfunction(skaffari_test _testname _link1 _link2 _link3)

function(skaffari_app_test _testname _link1 _link2 _link3)
    add_executable(${_testname}_exec ${_testname}.cpp)
#    add_test(NAME ${_testname} COMMAND ${_testname}_exec)
    target_link_libraries(${_testname}_exec ${_link1} ${_link2} ${_link3} skapp_test)
#    target_include_directories(${_testname}_exec SYSTEM PRIVATE ${Cutelyst2Qt5_INCLUDE_DIR})
endfunction(skaffari_app_test _testname _link1 _link2 _link3)

function(skaffari_web_test _testname _link1 _link2 _link3)
    add_executable(${_testname}_exec ${_testname}.cpp)
#    add_test(NAME ${_testname} COMMAND ${_testname}_exec)
    target_link_libraries(${_testname}_exec ${_link1} ${_link2} ${_link3} skweb_test)
#    target_include_directories(${_testname}_exec SYSTEM PRIVATE ${Cutelyst2Qt5_INCLUDE_DIR})
endfunction(skaffari_web_test _testname _link1 _link2 _link3)

skaffari_test(testdomain Cutelyst::Core Qt5::Network "")
skaffari_test(testaccount Cutelyst::Core "" "")
skaffari_test(testadminaccount Cutelyst::Core "" "")
skaffari_test(testemailaddress "" "" "")
skaffari_test(testfolder Qt5::Network "" "")
skaffari_test(testsimpleaccount Cutelyst::Core "" "")
skaffari_test(testsimpleadmin "" "" "")
skaffari_test(testsimpledomain "" "" "")

# ConfigChecker test
add_executable(testconfigchecker_exec
    testconfigchecker.cpp
    ../cmd/configchecker.h
    ../cmd/configchecker.cpp
    ../cmd/configfile.h
    ../cmd/configfile.cpp
    ../cmd/consoleoutput.h
    ../cmd/consoleoutput.cpp
    ../cmd/imap.h
    ../cmd/imap.cpp
    ../common/config.h)
add_test(NAME testconfigchecker COMMAND testconfigchecker_exec)
target_link_libraries(testconfigchecker_exec Qt5::Test Qt5::Network Qt5::Sql Cutelyst::Utils::Validator skaffari)
#target_include_directories(testconfigchecker_exec SYSTEM PRIVATE ${Cutelyst2Qt5_INCLUDE_DIR})

skaffari_app_test(testcmdsetup "" "" "")
skaffari_web_test(testwebui "" "" "")
