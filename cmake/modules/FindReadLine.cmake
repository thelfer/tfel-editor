# - Try to find readline include dirs and libraries 
#
# Usage of this module as follows:
#
#     find_package(Readline)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  READLINE_INSTALL_PATH         Set this variable to the root installation of
#                            readline if the module has problems finding the
#                            proper installation path.
#
# Variables defined by this module:
#
#  READLINE_INCLUDE_PATH The readline include directories. 
#  READLINE_LIBRARY      The readline library.

find_path(READLINE_INSTALL_PATH
    NAMES include/readline/readline.h
)

find_path(READLINE_INCLUDE_PATH
    NAMES readline/readline.h
    HINTS ${READLINE_INSTALL_PATH}/include
)

find_library(READLINE_LIBRARY
    NAMES readline
    HINTS ${READLINE_INSTALL_PATH}/lib
)

if(READLINE_INCLUDE_PATH AND READLINE_LIBRARY)
  message(STATUS "readline include  : ${READLINE_INCLUDE_PATH}")
  message(STATUS "readline libs     : ${READLINE_LIBRARY}")
else(READLINE_INCLUDE_PATH AND READLINE_LIBRARY)
  message(FATAL "libreadline not found")
endif(READLINE_INCLUDE_PATH AND READLINE_LIBRARY)

mark_as_advanced(
    READLINE_INSTALL_PATH
    READLINE_INCLUDE_PATH
    READLINE_LIBRARY
)