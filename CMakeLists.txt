
cmake_minimum_required(VERSION 3.16)

project(gas-generator)

set(CMAKE_CXX_STANDARD 17)

set(EXECUTABLE_NAME gas-cli)

# ROOT is not directly used in this project but Garfield requires it
find_package(ROOT REQUIRED)

include(FetchContent)

# Setting 'FIND_GARFIELD' to 'OFF' (-DFIND_GARFIELD=OFF) will prevent the build from looking for installed Garfield and will built it from source
if (NOT DEFINED FIND_GARFIELD)
    set(FIND_GARFIELD ON)
endif ()

# Garfield
if (FIND_GARFIELD)
    find_package(Garfield QUIET) # sets Garfield_FOUND to 1 (found) or 0 (not found)
endif ()
if (NOT Garfield_FOUND)
    set(GARFIELD_GIT_ID e0a9f171595b5281c61c12a6a1d1309af7acb950)
    message(STATUS "Garfield not found. Garfield will be built from source (Git ID ${GARFIELD_GIT_ID})")
    FetchContent_Declare(
            Garfield
            GIT_REPOSITORY https://gitlab.cern.ch/garfield/garfieldpp.git
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
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG ${nlohmann_json_GIT_ID}
    )
    FetchContent_MakeAvailable(nlohmann_json)
else ()
    message(STATUS "nlohmann_json found")
endif ()

add_executable(
        ${EXECUTABLE_NAME}
        main.cpp
)

file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
target_include_directories(${EXECUTABLE_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_sources(${EXECUTABLE_NAME} PRIVATE ${SOURCES})

# cli11 (CLI parser)
FetchContent_Declare(
        cli11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG v2.3.1
)
FetchContent_MakeAvailable(cli11)

target_link_libraries(
        ${EXECUTABLE_NAME}
        PUBLIC Garfield::Garfield nlohmann_json::nlohmann_json
        PRIVATE CLI11::CLI11
)

install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin)

add_subdirectory(tests EXCLUDE_FROM_ALL)
