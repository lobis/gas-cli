
message(STATUS "Google Test")

# https://google.github.io/googletest/quickstart-cmake.html
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) # Windows
FetchContent_MakeAvailable(googletest)

set(TESTING_EXECUTABLE "test${LIBRARY_NAME}")

enable_testing()

FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(${TESTING_EXECUTABLE} ${SOURCES})

target_link_libraries(
        ${TESTING_EXECUTABLE} PUBLIC
        ${LIBRARY_NAME}
        gtest_main
)

include(GoogleTest)

gtest_add_tests(
        TARGET ${TESTING_EXECUTABLE}
        SOURCES ${SOURCES}
)
