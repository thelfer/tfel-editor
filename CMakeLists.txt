include(cmake/modules/qemacs.cmake)
qemacs_project(qemacs 0 1-dev)
cmake_minimum_required(VERSION 2.4)

include(cmake/modules/autotools.cmake)

#compiler options
include(cmake/modules/compiler.cmake)
#find the tfel package
include(cmake/modules/tfel.cmake)

option(enable-tfel-plot "enable tfel-plot" OFF)
if(enable-tfel-plot)
  #find the tfel-plot package
  include(cmake/modules/tfel-plot.cmake)
endif(enable-tfel-plot)

# qt support
include(cmake/modules/qt.cmake)

# testing
set(CTEST_CONFIGURATION_TYPE "${JOB_BUILD_CONFIGURATION}")
# (must be placed *before* any add_subdirectory, cmake bug ?)
enable_testing()
if(CMAKE_CONFIGURATION_TYPES)
  add_custom_target(check COMMAND 
    ${CMAKE_CTEST_COMMAND} -T test -C $<CONFIGURATION>)
else(CMAKE_CONFIGURATION_TYPES)
  add_custom_target(check COMMAND 
    ${CMAKE_CTEST_COMMAND} -T test )
endif(CMAKE_CONFIGURATION_TYPES)

add_subdirectory(include)
add_subdirectory(src)
if(NOT WIN32)
add_subdirectory(packages)
endif(NOT WIN32)
