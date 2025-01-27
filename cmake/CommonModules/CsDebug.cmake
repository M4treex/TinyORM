# Print various internal CMake variables
function(cs_system_info)
    include(CMakePrintSystemInformation)
endfunction()

# Enable verbose output from Makefile builds
function(cs_verbose_makefile)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endfunction()

# Print all cmake variables
function(cs_print_vars)
    set(exclude_cmake yes)
    if(ARGC GREATER_EQUAL 1 AND DEFINED ARGV0 AND NOT ARGV0)
        set(exclude_cmake no)
        message(STATUS "All variables:")
    else()
        message(STATUS "All non-cmake variables:")
    endif()

    get_cmake_property(variable_names VARIABLES)
    foreach (variable ${variable_names})
        if(exclude_cmake AND "${variable}" MATCHES "(^(CMAKE_.*)|^(_.*))")
            continue()
        endif()

        message("${variable}=${${variable}}")
    endforeach()
endfunction()

# Print all env. variables
function(cs_print_env_vars)
    message(STATUS "All environment variables:")
    execute_process(COMMAND "${CMAKE_COMMAND}" "-E" "environment")
endfunction()

# Get all propreties that cmake supports
if(NOT CMAKE_PROPERTY_LIST)
    execute_process(
        COMMAND cmake --help-property-list
        OUTPUT_VARIABLE CMAKE_PROPERTY_LIST
    )

    # Convert command output into a CMake list
    string(REGEX REPLACE ";" "\\\\;" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")
    string(REGEX REPLACE "\n" ";" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")
endif()

# Print all target properties
function(cs_print_target_properties target)

    if(NOT TARGET ${target})
      message(FATAL_ERROR "There is no target named '${target}'")
    endif()

    message(STATUS "Target properties for '${target}':")

    foreach(property ${CMAKE_PROPERTY_LIST})
        string(REPLACE "<CONFIG>" "${CMAKE_BUILD_TYPE}" property ${property})

        if(property STREQUAL "LOCATION" OR property MATCHES "^LOCATION_"
                OR property MATCHES "_LOCATION$")
            continue()
        endif()

        get_property(was_set TARGET ${target} PROPERTY ${property} SET)
        if(was_set)
            get_target_property(value ${target} ${property})
            message("${property} = ${value}")
        endif()
    endforeach()

endfunction()

# Print clearly visible notice message about passed variable
function(p variable)
    message("|||-- ${variable} : ${${variable}}")
endfunction()

# Print status message about a passed variable
function(ps variable)
    message(STATUS "${variable} : ${${variable}}")
endfunction()

# Print notice message about a passed variable
function(pn variable)
    message(NOTICE "${variable} : ${${variable}}")
endfunction()
