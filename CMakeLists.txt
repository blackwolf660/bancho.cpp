cmake_minimum_required(VERSION 3.20)
project(bancho)
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# --- FetchContent ---
include(FetchContent)

FetchContent_Declare(asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG        asio-1-30-2
    GIT_SHALLOW    ON
)
FetchContent_MakeAvailable(asio)

set(ASIO_INCLUDE_DIR "${asio_SOURCE_DIR}/asio/include")

FetchContent_Declare(Crow
    GIT_REPOSITORY https://github.com/crowcpp/crow.git
    GIT_TAG v1.3.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(Crow)

FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW ON)
FetchContent_MakeAvailable(json)
# --- --- --- --- --- --- --- ---

set(Exes
    src/main.cpp
)
set(Libs
    Crow::Crow
    nlohmann_json::nlohmann_json
)

add_executable(${PROJECT_NAME} ${Exes})

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${asio_SOURCE_DIR}/asio/include
    "${CMAKE_SOURCE_DIR}/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${Libs})