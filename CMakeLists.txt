include(cmake/modules/tfel-editor.cmake)
tfel_editor_project(tfel-editor 1 1.0-dev)
cmake_minimum_required(VERSION 2.4)
cmake_policy(SET CMP0020 NEW)
include(cmake/modules/autotools.cmake)

#compiler options
include(cmake/modules/compiler.cmake)
#find the tfel package
include(cmake/modules/tfel.cmake)

option(enable-tfel-plot "enable tfel-plot" OFF)
if(enable-tfel-plot)
  # find the tfel-plot package
  include(cmake/modules/tfel-plot.cmake)
endif(enable-tfel-plot)

# qt support
include(cmake/modules/qt.cmake)

# hunspell support
include(cmake/modules/hunspell.cmake)

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
add_subdirectory(utils)
add_subdirectory(packages)
