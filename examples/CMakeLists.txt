# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
# jsonpp
find_package (Jsoncpp REQUIRED)
find_library(jsoncpp_LIBRARY jsoncpp) # why is this needed?

# -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
# boost
find_package(Boost COMPONENTS system program_options REQUIRED)

add_executable(l2l-cpp-commandline command-line.cpp)

target_include_directories(
  l2l-cpp-commandline PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${L2L-CPP_INCLUDE}
  ${Boost_INCLUDE_DIR})

target_link_libraries(l2l-cpp-commandline
  l2l-cpp
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${jsoncpp_LIBRARY})

target_compile_features(l2l-cpp-commandline PRIVATE "cxx_auto_type")