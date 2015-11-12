cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
find_package (Jsoncpp REQUIRED)

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
include(ExternalProject)

set(websocketpp_CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX=${websocketpp_INSTALL_DIR}
    -DENABLE_CPP11=1")

EXTERNALPROJECT_ADD(
    websocketpp
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
    URL "file://${PROJECT_SOURCE_DIR}/websocketpp-0.6.0.tar.gz"
    INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/websocketpp"
    CMAKE_ARGS ${websocketpp_CMAKE_ARGS}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND "")

ExternalProject_Get_Property(websocketpp SOURCE_DIR)
set(websocketpp_SOURCE_DIR "${SOURCE_DIR}")

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
add_library (l2l-cpp "l2l.cpp")

add_dependencies(l2l-cpp websocketpp)

target_include_directories (l2l-cpp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${websocketpp_SOURCE_DIR})
target_link_libraries(l2l-cpp ${jsoncpp_LIBRARY})

target_compile_features(l2l-cpp PRIVATE "cxx_auto_type")
