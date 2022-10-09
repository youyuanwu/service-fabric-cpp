# here are targets to fetch idls and c headers

# https://raw.githubusercontent.com/microsoft/service-fabric/master/src/prod/src/idl/public/FabricClient.idl

find_program (
    MIDL_exe
    NAMES midl.exe
    REQUIRED
)

set(idl_files
    FabricClient.idl
    FabricCommon.idl
    FabricRuntime.idl
    FabricTypes.idl
)

set(out_headers "")
set(out_srcs "")

# download idls
foreach(_idl_file ${idl_files})
    get_filename_component(_file_name ${_idl_file} NAME_WE)
    set(_idl_out_path ${CMAKE_CURRENT_BINARY_DIR}/${_idl_file})
    if(NOT EXISTS ${_idl_out_path})
        message(STATUS "downloading ${_idl_file}")
        file(DOWNLOAD
            https://raw.githubusercontent.com/microsoft/service-fabric/master/src/prod/src/idl/public/${_idl_file}
            ${_idl_out_path}
        )
    endif()

    set(_out_dir ${CMAKE_CURRENT_BINARY_DIR}/include)
    set(_out_header ${_out_dir}/${_file_name}.h)
    set(_out_src ${_out_dir}/${_file_name}_i.c)
    add_custom_command(
        OUTPUT ${_out_header} ${_out_src}
        BYPRODUCTS ${_out_dir}/${_file_name}_p.c ${_out_dir}/${_file_name}.tlb
        COMMAND ${MIDL_exe} ${_idl_out_path} /out ${_out_dir}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
    )
    list(APPEND out_headers ${_out_header})
    list(APPEND out_srcs ${_out_src})
endforeach()

# In idl only mode only downloads ids.
if(service_fabric_BuildIDLOnly)
  return()
endif()

# sf header only lib
add_library(fabric_headers INTERFACE ${out_headers})
target_include_directories(fabric_headers INTERFACE
    ${CMAKE_CURRENT_BINARY_DIR}/include # generated includes
)

# static lib that has uuids
add_library(fabric_uuids STATIC
    ${out_srcs}
)