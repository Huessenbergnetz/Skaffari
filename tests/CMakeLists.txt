project(skaffari_tests)

find_package(Qt5Test 5.6.0 REQUIRED)

function(skaffari_test _testname _link1 _link2 _link3)
    add_executable(${_testname}_exec ${_testname}.cpp)
    add_test(NAME ${_testname} COMMAND ${_testname}_exec)
    target_link_libraries(${_testname}_exec ${_link1} ${_link2} ${_link3} Qt5::Test skaffari)
    target_include_directories(${_testname}_exec PRIVATE ${Cutelyst2Qt5_INCLUDE_DIR})
endfunction(skaffari_test _testname _link1 _link2 _link3)

skaffari_test(testdomain "" "" "")
skaffari_test(testaccount "" "" "")
