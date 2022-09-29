
cmake_minimum_required(VERSION 3.16)

project(gas_generator)

set(CMAKE_CXX_STANDARD 17)

set(LIBRARY_NAME ${PROJECT_NAME})
set(EXECUTABLE_NAME ${PROJECT_NAME}-cli)

add_library(${LIBRARY_NAME} SHARED)

find_package(ROOT REQUIRED)

# Garfield
find_package(Garfield QUIET) # sets Garfield_FOUND to 1 (found) or 0 (not found)
if (NOT Garfield_FOUND)
    set(GARFIELD_HASH 0b769960696833d1f29f825d9e6a2295cfef6182)
    message(STATUS "Garfield not found. Garfield will be built from source (hash ${GARFIELD_HASH})")
    include(FetchContent)
    FetchContent_Declare(
            Garfield
            GIT_REPOSITORY https://gitlab.cern.ch/garfield/garfieldpp
            GIT_TAG ${GARFIELD_HASH}
    )
    set(WITH_EXAMPLES OFF) # Do not build Garfield with examples
    FetchContent_MakeAvailable(Garfield)
    unset(WITH_EXAMPLES) # Do not pollute the scope
else ()
    message(STATUS "Garfield found")
endif ()

# nlohmann json
find_package(nlohmann_json QUIET)
if (NOT nlohmann_json_FOUND)
    set(nlohmann_json_HASH v3.11.2)
    message(STATUS "nlohmann_json not found. nlohmann_json will be built from source (hash ${nlohmann_json_HASH})")
    include(FetchContent)
    FetchContent_Declare(
            nlohmann_json
            GIT_REPOSITORY https://github.com/nlohmann/json
            GIT_TAG ${nlohmann_json_HASH}
    )
    FetchContent_MakeAvailable(nlohmann_json)
else ()
    message(STATUS "nlohmann_json found")
endif ()

target_include_directories(${LIBRARY_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cpp)
target_sources(${LIBRARY_NAME} PRIVATE ${sources})
target_link_libraries(${LIBRARY_NAME} PUBLIC Garfield::Garfield PRIVATE nlohmann_json::nlohmann_json)

add_executable(
        ${EXECUTABLE_NAME}
        main.cpp
)

target_link_libraries(${EXECUTABLE_NAME} PUBLIC ${LIBRARY_NAME})
