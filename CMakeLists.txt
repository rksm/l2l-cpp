cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project (l2l-cpp)

find_program(CCACHE_FOUND ccache) 
if(CCACHE_FOUND) 
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache) 
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache) 
endif(CCACHE_FOUND) 
# set (CMAKE_CXX_FLAGS "-DPCL_ONLY_CORE_POINT_TYPES=ON -DNO_EXPLICIT_INSTANTIATIONS") 
 
find_package (Jsoncpp REQUIRED)
# why is this needed?
find_library(jsoncpp_LIBRARY jsoncpp)

find_package(Boost COMPONENTS system program_options REQUIRED)

include_directories("src" ${Boost_INCLUDE_DIR})

# include_directories("src src/websocketpp")
# include_directories("src")

set(l2l-cpp_SOURCES
  src/main.cpp
  src/l2l.cpp
)

add_executable(l2l-cpp ${l2l-cpp_SOURCES})

target_link_libraries(l2l-cpp
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${jsoncpp_LIBRARY})

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
# compiler features / version
target_compile_features(l2l-cpp PRIVATE "cxx_auto_type")
