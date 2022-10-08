# here are targets to build lib files

# find runtime and sdk location
if(NOT ServiceFabric_Runtime_ROOT)
    set(ServiceFabric_Runtime_ROOT "C:\\Program Files\\Microsoft Service Fabric")
endif()

set(ServiceFabric_Runtime_BINARY_DIR "${ServiceFabric_Runtime_ROOT}\\bin\\Fabric\\Fabric.Code")

# find dlls
find_file(FabricRuntime_DLL
    NAMES FabricRuntime.dll
    PATHS ${ServiceFabric_Runtime_BINARY_DIR}
    REQUIRED
)
message(STATUS "Found FabricRuntime: ${FabricRuntime_DLL}")

find_file(FabricClient_DLL
    NAMES FabricClient.dll
    PATHS ${ServiceFabric_Runtime_BINARY_DIR}
    REQUIRED
)
message(STATUS "Found FabricClient: ${FabricClient_DLL}")

find_file(FabricResources_DLL
    NAMES FabricResources.dll
    PATHS ${ServiceFabric_Runtime_BINARY_DIR}
    REQUIRED
)
message(STATUS "Found FabricResources: ${FabricResources_DLL}")

# find dumpbin
find_program (
    Dumpbin_exe
    NAMES dumpbin.exe
    REQUIRED
)

# generate libs
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/FabricRuntime.lib
    COMMAND ${service-fabric_SOURCE_DIR}/scripts/dll2lib.bat 64 ${FabricRuntime_DLL}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM
)

add_library(FabricRuntime SHARED IMPORTED)
# set_target_properties(FabricRuntime PROPERTIES IMPORTED_IMPLIB ${CMAKE_CURRENT_BINARY_DIR}/FabricRuntime.lib )
# target_link_directories(FabricRuntime
#     INTERFACE ${CMAKE_CURRENT_BINARY_DIR}
# )

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/FabricClient.lib
    COMMAND ${service-fabric_SOURCE_DIR}/scripts/dll2lib.bat 64 ${FabricClient_DLL}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM
)

add_library(FabricClient SHARED IMPORTED)
# path set is not working. ideally this should be fixed.
#set_target_properties(FabricClient PROPERTIES IMPORTED_IMPLIB ${CMAKE_CURRENT_BINARY_DIR}/FabricClient.lib )
# set_target_properties(FabricClient PROPERTIES
#     IMPORTED_LOCATION ${FabricClient_DLL}
#     IMPORTED_IMPLIB ${CMAKE_CURRENT_BINARY_DIR}/FabricClient.lib
# )

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/FabricResources.lib
    COMMAND ${service-fabric_SOURCE_DIR}/scripts/dll2lib.bat 64 ${FabricResources_DLL}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM
)

add_custom_target(fabric_libs ALL
    DEPENDS 
    ${CMAKE_CURRENT_BINARY_DIR}/FabricRuntime.lib
    ${CMAKE_CURRENT_BINARY_DIR}/FabricClient.lib
    ${CMAKE_CURRENT_BINARY_DIR}/FabricResources.lib
)
