# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2021-2025, The OpenROAD Authors

# Generate the messages.txt file automatically
# Arguments
#   TARGET <target>: the target to post-build trigger from [required]
#   OUTPUT_DIR <dir>: the directory to write the messages.txt in
#                     [defaults to .]
#   SOURCE_DIR <dir>: the directory to search for sources from
#                     [defaults to OUTPUT_DIR]
#   LOCAL: don't recurse [defaults to false]

function(messages)

  # Parse args
  set(options LOCAL)
  set(oneValueArgs TARGET OUTPUT_DIR SOURCE_DIR)
  set(multiValueArgs "")

  cmake_parse_arguments(
      ARG  # prefix on the parsed args
      "${options}"
      "${oneValueArgs}"
      "${multiValueArgs}"
      ${ARGN}
  )

  # Validate args
  if (DEFINED ARG_UNPARSED_ARGUMENTS)
     message(FATAL_ERROR "Unknown argument(s) to swig_lib: ${ARG_UNPARSED_ARGUMENTS}")
  endif()

  if (DEFINED ARG_KEYWORDS_MISSING_VALUES)
     message(FATAL_ERROR "Missing value for argument(s) to swig_lib: ${ARG_KEYWORDS_MISSING_VALUES}")
  endif()

  if (NOT DEFINED ARG_TARGET)
    message(FATAL_ERROR "TARGET argument must be provided to messages")
  endif()

  if (DEFINED ARG_OUTPUT_DIR)
    set(OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_OUTPUT_DIR})
  else()
    set(OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  if (DEFINED ARG_SOURCE_DIR)
    set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_SOURCE_DIR})
  else()
    set(SOURCE_DIR ${OUTPUT_DIR})
  endif()

  set(glob "GLOB_RECURSE")
  if (${ARG_LOCAL})
    set(local '--local')
    set(glob "GLOB")
  endif()

  get_target_property(target_type ${ARG_TARGET} TYPE)
  if (${target_type} STREQUAL "INTERFACE_LIBRARY")
    get_target_property(library_list ${ARG_TARGET} INTERFACE_LINK_LIBRARIES)
  else()
    get_target_property(library_list ${ARG_TARGET} LINK_LIBRARIES)
  endif()
  if ("${library_list}" STREQUAL "library_list-NOTFOUND")
    set(library_list "")
  endif()

  # Search for dependencies within the same tool module
  set(dependency_list "")
  file(${glob} makefile_list "${SOURCE_DIR}/CMakeLists.txt")
  foreach(library ${library_list})
    set(library_found FALSE)
    foreach(FILE ${makefile_list})
      execute_process(
        COMMAND bash -c "grep 'add_library\\s*(\\s*${library}' ${FILE}"
        OUTPUT_VARIABLE output
      )
      if(NOT "${output}" STREQUAL "")
        set(library_found TRUE)
        break()
      endif()
    endforeach()
    if(${library_found})
      list(APPEND dependency_list ${library})
    endif()
  endforeach()

  if ("${dependency_list}" STREQUAL "")
    add_custom_command(
      TARGET ${ARG_TARGET}
      POST_BUILD
      COMMAND ${CMAKE_SOURCE_DIR}/etc/find_messages.py
        ${local}
        > ${OUTPUT_DIR}/messages.txt
      WORKING_DIRECTORY ${SOURCE_DIR}
    )
  else()
    add_custom_command(
      OUTPUT messages.checked
      COMMAND ${CMAKE_SOURCE_DIR}/etc/find_messages.py
        ${local}
        > ${OUTPUT_DIR}/messages.txt
	&& touch ${OUTPUT_DIR}/messages.checked
      WORKING_DIRECTORY ${SOURCE_DIR}
      DEPENDS ${dependency_list}
    )
    add_custom_target(${ARG_TARGET}_messages DEPENDS messages.checked)
    add_dependencies(${ARG_TARGET} ${ARG_TARGET}_messages)
  endif()

endfunction()
