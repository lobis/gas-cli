
cmake_minimum_required(VERSION 3.16)

project(gas-generator)

set(CMAKE_CXX_STANDARD 17)

set(LIBRARY_NAME Gas)
set(EXECUTABLE_NAME gas-cli)

add_library(${LIBRARY_NAME} SHARED)

find_package(ROOT REQUIRED)

include(FetchContent)

# Garfield
find_package(Garfield QUIET) # sets Garfield_FOUND to 1 (found) or 0 (not found)
if (NOT Garfield_FOUND)
    set(GARFIELD_GIT_ID 0b769960696833d1f29f825d9e6a2295cfef6182)
    message(STATUS "Garfield not found. Garfield will be built from source (Git ID ${GARFIELD_GIT_ID})")
    FetchContent_Declare(
            Garfield
            GIT_REPOSITORY https://gitlab.cern.ch/garfield/garfieldpp
            GIT_TAG ${GARFIELD_GIT_ID}
    )
    set(WITH_EXAMPLES OFF) # Do not build Garfield with examples
    FetchContent_MakeAvailable(Garfield)
    unset(WITH_EXAMPLES) # Do not pollute the scope
else ()
    message(STATUS "Garfield found (${Garfield_VERSION})")
endif ()

# nlohmann json
find_package(nlohmann_json QUIET)
if (NOT nlohmann_json_FOUND)
    set(nlohmann_json_GIT_ID v3.11.2)
    message(STATUS "nlohmann_json not found. nlohmann_json will be built from source (Git ID ${nlohmann_json_GIT_ID})")
    include(FetchContent)
    FetchContent_Declare(
            nlohmann_json
            GIT_REPOSITORY https://github.com/nlohmann/json
            GIT_TAG ${nlohmann_json_GIT_ID}
    )
    FetchContent_MakeAvailable(nlohmann_json)
else ()
    message(STATUS "nlohmann_json found")
endif ()

# cli11 (CLI parser)
FetchContent_Declare(
        cli11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11
        GIT_TAG v2.2.0
)
FetchContent_MakeAvailable(cli11)

target_include_directories(${LIBRARY_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cpp)
target_sources(${LIBRARY_NAME} PRIVATE ${sources})
target_link_libraries(${LIBRARY_NAME} PUBLIC Garfield::Garfield PRIVATE nlohmann_json::nlohmann_json)

add_executable(
        ${EXECUTABLE_NAME}
        main.cpp
)

target_link_libraries(${EXECUTABLE_NAME} PUBLIC ${LIBRARY_NAME} PRIVATE CLI11::CLI11)

install(TARGETS ${LIBRARY_NAME} DESTINATION lib)
install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin)

add_subdirectory(tests EXCLUDE_FROM_ALL)
