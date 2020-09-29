cmake_minimum_required (VERSION 3.5 FATAL_ERROR)

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# mac-address
set(app mac-address)

project (${app} C CXX)

set(Soruces
    ${app}.cpp
)

set(Headers
    ${app}.hpp
)

# Create executable file
add_executable(${app} ${Soruces} ${Headers})
target_link_libraries (${app} -lstdc++fs)

install(TARGETS ${app} DESTINATION bin)

# Systemd service
set(SYSTEMD_TARGET multi-user.target)
configure_file (${app}.service.in ${app}.service)

option (ENABLE_TEST "Enable Google Test" ON)
# Test unit of mac-address
if (ENABLE_TEST)
    set(testapp testMacAddress)

    set (
        utSoruces test/${testapp}.cpp
    )

    enable_testing ()

    add_executable (${testapp} ${utSoruces})
    target_link_libraries (${testapp} gtest gtest_main -lpthread stdc++fs)

    add_test (NAME ${testapp} COMMAND ${testapp} )
endif ()
