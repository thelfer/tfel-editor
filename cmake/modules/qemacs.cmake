macro(qemacs_project qemacs_version_major qemacs_version_minor)
  project("tfel-plot")
  set(PACKAGE_NAME "tfel-plot")
  set(VERSION "${qemacs_version_major}.${qemacs_version_minor}")
  # the version number.
  set(QEMACS_VERSION_MAJOR "${qemacs_version_major}")
  set(QEMACS_VERSION_MINOR "${qemacs_version_minor}")
  add_definitions("-DVERSION=\\\"\"${qemacs_version_major}.${qemacs_version_minor}\"\\\"")
endmacro(qemacs_project)

set(PACKAGE_BUGREPORT "tfel-contact@cea.fr")
add_definitions("-DPACKAGE_BUGREPORT=\\\"\"${PACKAGE_BUGREPORT}\"\\\"")
add_definitions("-DPACKAGE_NAME=\\\"\"${PACKAGE_NAME}\"\\\"")
add_definitions("-DPACKAGE=\\\"\"${PACKAGE_NAME}\"\\\"")
add_definitions("-DPACKAGE_VERSION=\\\"\"${qemacs_version_major}.${qemacs_version_minor}\"\\\"")

macro(install_header dir file)
  install(FILES ${dir}/${file}
          DESTINATION "include/${dir}")
endmacro(install_header)

macro(install_data dir file)
  install(FILES ${file}
          DESTINATION "share/${PACKAGE_NAME}/${dir}")
endmacro(install_data)

macro(qemacs_library name)
if(${ARGC} LESS 1)
   message(FATAL_ERROR "qemacs_library : no source specified")
endif(${ARGC} LESS 1)
add_library(${name} SHARED ${ARGN})
install(TARGETS ${name} DESTINATION lib)
if(ENABLE_STATIC)
 add_library(${name}-static STATIC ${ARGN})
 set_target_properties(${name}-static PROPERTIES OUTPUT_NAME "${name}")
 # Now the library target "${name}-static" will be named "${name}.lib"
 # with MS tools.
 # This conflicts with the "${name}.lib" import library corresponding
 # to "${name}.dll",
 # so we add a "lib" prefix (which is default on other platforms
 # anyway):
 set_target_properties(${name}-static PROPERTIES PREFIX "lib")
 # Help CMake 2.6.x and lower (not necessary for 2.8 and above, but
 # doesn't hurt):
 set_target_properties(${name}        PROPERTIES CLEAN_DIRECT_OUTPUT 1)
 set_target_properties(${name}-static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
 install(TARGETS ${name}-static DESTINATION lib)
endif(ENABLE_STATIC)
endmacro(qemacs_library)

macro(qemacs_executable name)
if(${ARGC} LESS 1)
   message(FATAL_ERROR "qemacs_executable : no source specified")
endif(${ARGC} LESS 1)
add_executable(${name} ${ARGN})
install(TARGETS ${name} DESTINATION bin)
endmacro(qemacs_executable)

set(prefix           "${CMAKE_INSTALL_PREFIX}")

macro(qemacs_package name)
  if(${ARGC} LESS 1)
    message(FATAL_ERROR "qemacs_package : no source specified")
  endif(${ARGC} LESS 1)
  add_library(${name} SHARED ${ARGN})
  install(TARGETS ${name} DESTINATION share/qemacs/packages)
endmacro(qemacs_package)
