
if(NOT ServiceFabric_Runtime_ROOT)
    set(ServiceFabric_Runtime_ROOT "C:\\Program Files\\Microsoft Service Fabric")
endif()

set(ServiceFabric_Runtime_BINARY_DIR "${ServiceFabric_Runtime_ROOT}\\bin\\Fabric\\Fabric.Code")

# this function generates from import lib from a dll
function(add_fabric_lib)
    cmake_parse_arguments(
        FABRIC_LIB # prefix of output variables
        "" # list of names of the boolean arguments (only defined ones will be true)
        "NAME;OUTDIR;DLLDIR" # list of names of mono-valued arguments
        #"SRCS;DEPS" # list of names of multi-valued arguments (output variables are lists)
        ""
        ${ARGN} # arguments of the function to parse, here we take the all original ones
    )

    if(NOT FABRIC_LIB_NAME)
        message(FATAL_ERROR "Name param not found")
    endif(NOT FABRIC_LIB_NAME)

    if(NOT FABRIC_LIB_OUTDIR)
        message(FATAL_ERROR "Name param not found")
    endif(NOT FABRIC_LIB_OUTDIR)

    if(NOT FABRIC_LIB_DLLDIR)
        message(FATAL_ERROR "Name param not found")
    endif(NOT FABRIC_LIB_DLLDIR)
    # message("Provided sources are:")
    # foreach(src ${PARSED_ARGS_SRCS})
    #     message("- ${src}")
    # endforeach(src)
    set(_lib_var "${FABRIC_LIB_NAME}_DLL")
    set(_lib_file "${FABRIC_LIB_NAME}.dll")
    set(_out_file "${FABRIC_LIB_NAME}.lib")
    find_file(${_lib_var}
        NAMES ${_lib_file}
        PATHS ${FABRIC_LIB_DLLDIR}
        REQUIRED
    )
    message(STATUS "Found ${FABRIC_LIB_NAME}: ${${_lib_var}}")

    # generate fabric import lib
    add_custom_command(
        OUTPUT ${FABRIC_LIB_OUTDIR}/${_out_file}
        COMMAND ${CMAKE_SOURCE_DIR}/scripts/dll2lib.bat 64 "${${_lib_var}}"
        WORKING_DIRECTORY ${FABRIC_LIB_OUTDIR}
        VERBATIM
    )

    add_library(${FABRIC_LIB_NAME} SHARED IMPORTED GLOBAL)

    #set_target_properties(FabricClient PROPERTIES IMPORTED_IMPLIB ${CMAKE_CURRENT_BINARY_DIR}/FabricClient.lib )
    set_target_properties(${FABRIC_LIB_NAME} PROPERTIES
        IMPORTED_LOCATION ${_lib_var}
        IMPORTED_IMPLIB ${FABRIC_LIB_OUTDIR}/${_out_file}
    )

    set(_generate_target_name "generate_${FABRIC_LIB_NAME}_lib")
    add_custom_target(${_generate_target_name}
        DEPENDS 
        ${FABRIC_LIB_OUTDIR}/${_out_file}
    )

    add_dependencies(${FABRIC_LIB_NAME} ${_generate_target_name})
endfunction(add_fabric_lib)