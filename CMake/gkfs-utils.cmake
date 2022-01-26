################################################################################
# Copyright 2018-2021, Barcelona Supercomputing Center (BSC), Spain            #
# Copyright 2015-2021, Johannes Gutenberg Universitaet Mainz, Germany          #
#                                                                              #
# This software was partially supported by the                                 #
# EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).    #
#                                                                              #
# This software was partially supported by the                                 #
# ADA-FS project under the SPPEXA project funded by the DFG.                   #
#                                                                              #
# This file is part of GekkoFS.                                                #
#                                                                              #
# GekkoFS is free software: you can redistribute it and/or modify              #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation, either version 3 of the License, or            #
# (at your option) any later version.                                          #
#                                                                              #
# GekkoFS is distributed in the hope that it will be useful,                   #
# but WITHOUT ANY WARRANTY; without even the implied warranty of               #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                #
# GNU General Public License for more details.                                 #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with GekkoFS.  If not, see <https://www.gnu.org/licenses/>.            #
#                                                                              #
# SPDX-License-Identifier: GPL-3.0-or-later                                    #
################################################################################

include(CMakeParseArguments)

#[=======================================================================[.rst:

  get_cmake_variables(OUTPUT_VARIABLE
                     [ REGEX <regular_expression> [EXCLUDE] ])

Initialize ``OUTPUT_VARIABLE`` to a list of all currently defined CMake
variables. The function accepts a ``<regular_expression>`` to allow filtering
the results. Furthermore, if the ``EXCLUDE`` flag is used, the function will
return all variables NOT MATCHING the provided ``<regular_expression>``.
#]=======================================================================]
function(get_cmake_variables OUTPUT_VARIABLE)

  set(OPTIONS EXCLUDE)
  set(SINGLE_VALUE REGEX)
  set(MULTI_VALUE) # no multiple value args for now

  cmake_parse_arguments(
    ARGS "${OPTIONS}" "${SINGLE_VALUE}" "${MULTI_VALUE}" ${ARGN}
  )

  if(ARGS_UNPARSED_ARGUMENTS)
    message(WARNING "Unparsed arguments in get_cmake_variables(): "
                    "this often indicates typos!\n"
                    "Unparsed arguments: ${ARGS_UNPARSED_ARGUMENTS}"
    )
  endif()

  get_cmake_property(_var_names VARIABLES)

  if(NOT ARGS_REGEX)
    set(${OUTPUT_VARIABLE}
        ${_var_names}
        PARENT_SCOPE
    )
    return()
  endif()

  if(ARGS_EXCLUDE)
    set(_mode EXCLUDE)
  else()
    set(_mode INCLUDE)
  endif()

  list(FILTER _var_names ${_mode} REGEX ${ARGS_REGEX})
  set(${OUTPUT_VARIABLE}
      ${_var_names}
      PARENT_SCOPE
  )
endfunction()

#[=======================================================================[.rst:

  dump_cmake_variables([ REGEX <regular_expression> [EXCLUDE] ])

Print all currently defined CMake variables. The function accepts a
``<regular_expression>`` to allow filtering the results. Furthermore, if the
``EXCLUDE`` flag is used, the function will print all variables NOT MATCHING
the provided ``<regular_expression>``.
#]=======================================================================]
function(dump_cmake_variables)

  set(OPTIONS EXCLUDE)
  set(SINGLE_VALUE REGEX)
  set(MULTI_VALUE) # no multiple value args for now

  cmake_parse_arguments(
    ARGS "${OPTIONS}" "${SINGLE_VALUE}" "${MULTI_VALUE}" ${ARGN}
  )

  if(ARGS_UNPARSED_ARGUMENTS)
    message(WARNING "Unparsed arguments in dump_cmake_variables(): "
                    "this often indicates typos!"
                    "Unparsed arguments: ${ARGS_UNPARSED_ARGUMENTS}"
    )
  endif()

  if(ARGS_EXCLUDE AND NOT ARGS_REGEX)
    message(ERROR "EXCLUDE option doesn't make sense without REGEX.")
  endif()

  get_cmake_variables(_var_names REGEX ${ARGS_REGEX} ${ARGS_EXCLUDE})

  foreach(_var ${_var_names})
    message(STATUS "${_var}=${${_var}}")
  endforeach()
endfunction()

#[=======================================================================[.rst:

  mark_variables_as_advanced(REGEX <regular_expression>)

Mark all CMake variables matching ``regular_expression`` as advanced.
#]=======================================================================]
function(mark_variables_as_advanced)

  set(OPTIONS) # no options for now
  set(SINGLE_VALUE REGEX)
  set(MULTI_VALUE) # no multiple value args for now

  cmake_parse_arguments(
    ARGS "${OPTIONS}" "${SINGLE_VALUE}" "${MULTI_VALUE}" ${ARGN}
  )

  if(ARGS_UNPARSED_ARGUMENTS)
    message(WARNING "Unparsed arguments in mark_variables_as_advanced(): "
                    "this often indicates typos!\n"
                    "Unparsed arguments: ${ARGS_UNPARSED_ARGUMENTS}"
    )
  endif()

  get_cmake_property(_var_names VARIABLES)

  list(FILTER _var_names INCLUDE REGEX ${ARGS_REGEX})

  foreach(_var ${_var_names})
    mark_as_advanced(${_var})
  endforeach()
endfunction()
