# Unit tests for the platform-independent modules, run on the host with CTest.
# Unity is fetched at configure time (the CI runner and a normal dev box both
# have network access); pinned to a release tag for reproducibility.
include(FetchContent)

FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG v2.6.0
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(unity)

# Helper: declare a Unity-based test executable linked against the core library.
function(add_unit_test name)
    add_executable(${name} ${name}.c ${ARGN})
    target_link_libraries(${name} PRIVATE acmeter_core unity)
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_unit_test(test_rms)
add_unit_test(test_freq)
add_unit_test(test_dfdt)
add_unit_test(test_harmonic)
