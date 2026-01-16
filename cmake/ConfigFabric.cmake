include(cmake/ImportFabric.cmake)

# find midl exe
find_program(MIDL_exe
    NAMES midl.exe
    REQUIRED
)

# glob all idl files in idl folder
set(idl_dir ${fabric_metadata_SOURCE_DIR}/idl)
file(GLOB idl_files
    ${idl_dir}/*.idl
)

set(out_headers "")
set(out_srcs "")

foreach(_idl_file ${idl_files})
    get_filename_component(_file_name ${_idl_file} NAME_WE)
    set(_idl_src_path ${_idl_file})

    set(_out_dir ${CMAKE_CURRENT_BINARY_DIR}/gen)
    set(_out_header ${_out_dir}/${_file_name}.h)
    set(_out_src ${_out_dir}/${_file_name}_i.c)
    add_custom_command(
        OUTPUT ${_out_header} ${_out_src}
        COMMAND ${MIDL_exe} /no_settings_comment /utf8 /sal /sal_local /I ${idl_dir} ${_idl_src_path} /out ${_out_dir}
        # remove unused outfile
        COMMAND ${CMAKE_COMMAND} -E rm -f ${_out_dir}/${_file_name}_p.c ${_out_dir}/${_file_name}.tlb ${_out_dir}/dlldata.c
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
    )
    list(APPEND out_headers ${_out_header})
    list(APPEND out_srcs ${_out_src})
endforeach()

# define the sf_sdk target
add_library(fabric_sdk STATIC ${out_headers} ${out_srcs})
target_include_directories(fabric_sdk PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/gen)


# glob all internal idl in internal_idl folder
set(internal_idl_dir ${fabric_metadata_SOURCE_DIR}/internal_idl)
file(GLOB internal_idl_files
    ${internal_idl_dir}/*.idl
)

set(internal_out_headers "")
set(internal_out_srcs "")
foreach(_idl_file ${internal_idl_files})
    get_filename_component(_file_name ${_idl_file} NAME_WE)
    set(_idl_src_path ${_idl_file})

    set(_out_dir ${CMAKE_CURRENT_BINARY_DIR}/gen_internal)
    set(_out_header ${_out_dir}/${_file_name}.h)
    set(_out_src ${_out_dir}/${_file_name}_i.c)
    add_custom_command(
        OUTPUT ${_out_header} ${_out_src}
        COMMAND ${MIDL_exe} /no_settings_comment /utf8 /sal /sal_local /I ${internal_idl_dir} /I ${idl_dir} ${_idl_src_path} /out ${_out_dir}
        # remove unused outfile
        COMMAND ${CMAKE_COMMAND} -E rm -f ${_out_dir}/${_file_name}_p.c ${_out_dir}/${_file_name}.tlb ${_out_dir}/dlldata.c
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
    )
    list(APPEND internal_out_headers ${_out_header})
    list(APPEND internal_out_srcs ${_out_src})
endforeach()

# define the sf_internal_sdk target
add_library(fabric_internal_sdk STATIC ${internal_out_headers} ${internal_out_srcs})
target_include_directories(fabric_internal_sdk PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/gen_internal)

# add fabric import libs

set(FABRIC_LIB_NAMES
  FabricCommon
  FabricRuntime
  FabricClient
  FabricResources
  FabricServiceCommunication
  FabricTransport
)

foreach(_fabric_lib_name ${FABRIC_LIB_NAMES})
  add_fabric_lib(
    NAME ${_fabric_lib_name}
    OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/sf_importlibs
    DLLDIR ${ServiceFabric_Runtime_BINARY_DIR}
  )
endforeach()

add_custom_target(generate_importlibs
  DEPENDS
  # generate all support libs. 
  FabricCommon
  FabricRuntime
  FabricClient
  FabricResources
  FabricServiceCommunication
  FabricTransport
)