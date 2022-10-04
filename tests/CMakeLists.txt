
message(STATUS "Google Test")

# https://google.github.io/googletest/quickstart-cmake.html
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) # Windows
FetchContent_MakeAvailable(googletest)

set(TESTING_EXECUTABLE "tests")

enable_testing()

FILE(GLOB TESTING_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB EXECUTABLE_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(${TESTING_EXECUTABLE} ${TESTING_SOURCES})

target_include_directories(${TESTING_EXECUTABLE} PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_sources(${TESTING_EXECUTABLE} PRIVATE ${EXECUTABLE_SOURCES})

target_link_libraries(
        ${TESTING_EXECUTABLE} PRIVATE
        PUBLIC Garfield::Garfield
        PRIVATE nlohmann_json::nlohmann_json gtest_main
)

include(GoogleTest)

gtest_add_tests(
        TARGET ${TESTING_EXECUTABLE}
        SOURCES ${TESTING_SOURCES}
)
