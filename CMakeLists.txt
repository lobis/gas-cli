
cmake_minimum_required(VERSION 3.16)

project(gas_generator)

set(CMAKE_CXX_STANDARD 17)

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

add_executable(
        ${PROJECT_NAME}
        main.cpp
)

target_link_libraries(${PROJECT_NAME} Garfield::Garfield)
