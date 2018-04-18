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

function(skaffari_test _testname _link1 _link2 _link3)
    add_executable(${_testname}_exec ${_testname}.cpp)
    add_test(NAME ${_testname} COMMAND ${_testname}_exec)
    target_link_libraries(${_testname}_exec ${_link1} ${_link2} ${_link3} Qt5::Test skaffari)
    target_include_directories(${_testname}_exec PRIVATE ${Cutelyst2Qt5_INCLUDE_DIR})
endfunction(skaffari_test _testname _link1 _link2 _link3)

set(SKAFFARI_CMD_EXE "${CMAKE_BINARY_DIR}/cmd/skaffaricmd")

function(skaffari_app_test _testname _link1 _link2 _link3)
    add_executable(${_testname}_exec ${_testname}.cpp)
#    add_test(NAME ${_testname} COMMAND ${_testname}_exec)
    target_link_libraries(${_testname}_exec ${_link1} ${_link2} ${_link3} skapp_test)
    target_include_directories(${_testname}_exec PRIVATE ${Cutelyst2Qt5_INCLUDE_DIR})
    target_compile_definitions(${_testname}_exec
        PRIVATE
            SKAFFARI_CMD="${SKAFFARI_CMD_EXE}"
    )
endfunction(skaffari_app_test _testname _link1 _link2 _link3)

skaffari_test(testdomain "" "" "")
skaffari_test(testaccount "" "" "")
skaffari_test(testadminaccount Cutelyst::Core "" "")
skaffari_test(testemailaddress "" "" "")
skaffari_test(testfolder "" "" "")
skaffari_test(testsimpleaccount "" "" "")
skaffari_test(testsimpleadmin "" "" "")
skaffari_test(testsimpledomain "" "" "")

skaffari_app_test(testcmdsetup "" "" "")
