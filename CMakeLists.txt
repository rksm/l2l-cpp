cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project (l2l-cpp)

# caching
find_program(CCACHE_FOUND ccache) 
if(CCACHE_FOUND) 
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache) 
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache) 
endif(CCACHE_FOUND) 
# set (CMAKE_CXX_FLAGS "-DPCL_ONLY_CORE_POINT_TYPES=ON -DNO_EXPLICIT_INSTANTIATIONS") 

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
# websocketpp
include(ExternalProject)

set(websocketpp_CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${websocketpp_INSTALL_DIR} -DENABLE_CPP11=1")

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
# jsonpp
find_package (Jsoncpp REQUIRED)
find_library(jsoncpp_LIBRARY jsoncpp) # why is this needed?

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
# boost
find_package(Boost COMPONENTS system program_options REQUIRED)

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
# build
include_directories("src" ${websocketpp_SOURCE_DIR} ${Boost_INCLUDE_DIR})

set(l2l-cpp_SOURCES
  src/main.cpp
  src/l2l.cpp
)

add_executable(l2l-cpp ${l2l-cpp_SOURCES})

add_dependencies(l2l-cpp websocketpp)

target_link_libraries(l2l-cpp
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${jsoncpp_LIBRARY})

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
# compiler features / version
target_compile_features(l2l-cpp PRIVATE "cxx_auto_type")
