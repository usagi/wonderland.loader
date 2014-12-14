cmake_minimum_required(VERSION 2.8.12)

project(example)

if(CMAKE_CXX_COMPILER MATCHES "/em\\+\\+(-[a-zA-Z0-9.])?$")
  message(" * C++ compiler: Emscripten")
  set(CMAKE_CXX_COMPILER_ID "Emscripten")
else()
  message(" * C++ cimpiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

include_directories(${PROJECT_SOURCE_DIR}/../include)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Emscripten")
  find_package (Threads)
  add_executable(example main.cxx)
  target_link_libraries(example ${CMAKE_THREAD_LIBS_INIT})
  target_link_libraries(example curlpp curl)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s DEMANGLE_SUPPORT=1")
  add_executable(example.html main.cxx)
endif()

