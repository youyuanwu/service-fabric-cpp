
find_program(CPPWINRT 
  NAMES cppwinrt.exe
  REQUIRED
)

# winmdidl.exe /nologo /out:output.idl input.winmd

find_program(winmdidl_exe
  NAMES winmdidl.exe
  REQUIRED
)

find_program(midlrt_exe
  NAMES midlrt.exe
  REQUIRED
)

# add_custom_target(generate_winrt
#   COMMAND ${CPPWINRT} -input ${CMAKE_SOURCE_DIR}/build/_deps/fabric_metadata-src/.windows/winmd/Microsoft.ServiceFabric.winmd
#     -reference ${CMAKE_SOURCE_DIR}/build/_deps/fabric_metadata-src/.windows/winmd/Windows.Win32.winmd
#     -output ${CMAKE_CURRENT_SOURCE_DIR}
#     -component . -overwrite
# )

add_custom_target(generate_winrt_base_idl
  COMMAND ${winmdidl_exe} /nologo 
    /outdir:${CMAKE_CURRENT_SOURCE_DIR}/idlbase ${CMAKE_SOURCE_DIR}/build/_deps/fabric_metadata-src/.windows/winmd/Windows.Win32.winmd
    /metadata_dir:C:/Windows/System32/WinMetadata
)

add_custom_target(generate_winrt_idl
  COMMAND ${winmdidl_exe} /nologo 
    /outdir:${CMAKE_CURRENT_SOURCE_DIR}/idl ${CMAKE_SOURCE_DIR}/build/_deps/fabric_metadata-src/.windows/winmd/Microsoft.ServiceFabric.winmd
    /metadata_dir:${CMAKE_SOURCE_DIR}/build/_deps/fabric_metadata-src/.windows/winmd
)

add_custom_target(generate_winrt_header
  COMMAND ${midlrt_exe} /out ${CMAKE_CURRENT_SOURCE_DIR}/src /h myheader.h idl/Microsoft.ServiceFabric.idl /I idlbase2
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)