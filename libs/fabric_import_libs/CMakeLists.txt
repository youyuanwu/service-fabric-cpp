# here are targets to build lib files

set(FABRIC_LIB_NAMES
    FabricRuntime
    FabricClient
    FabricResources
)

include(ImportFabric)
foreach(_fabric_lib_name ${FABRIC_LIB_NAMES})
    add_fabric_lib(NAME ${_fabric_lib_name})
endforeach()