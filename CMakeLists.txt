cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project (l2l-cpp)

# caching
find_program(CCACHE_FOUND ccache) 
if(CCACHE_FOUND) 
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache) 
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache) 
endif(CCACHE_FOUND) 
# set (CMAKE_CXX_FLAGS "-DPCL_ONLY_CORE_POINT_TYPES=ON -DNO_EXPLICIT_INSTANTIATIONS") 

option (BUILD_EXAMPLES "Build websocketpp examples." FALSE)

if (BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

add_subdirectory(src)
