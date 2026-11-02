cmake_minimum_required(VERSION 3.10)

include(cmake/modules/tfel-editor.cmake)
tfel_editor_project(tfel-editor 1 1.0-dev)
cmake_policy(SET CMP0020 NEW)
include(cmake/modules/autotools.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)

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

#documentations
add_custom_target(doc)

# add a target to generate API documentation with Doxygen
option(enable-doxygen-doc "enable generation of the Doxygen documentation" OFF)
if(enable-doxygen-doc)
  find_package(Doxygen)
  if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
      ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    add_custom_target(doc-html
      ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen" VERBATIM)
    add_dependencies(doc doc-html)
    set(GENERATE_DOXYGEN ON)
  else(DOXYGEN_FOUND)
    message(FATAL_ERROR "doxygen is required")
  endif(DOXYGEN_FOUND)
else(enable-doxygen-doc)
  message(STATUS "doxygen documentation is not enabled")
endif(enable-doxygen-doc)

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

add_subdirectory(docs)
add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(utils)
add_subdirectory(packages)
