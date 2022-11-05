param(
    [String]
    $Source,
    [String]
    $OutFile
)

# coan reports 0x11 error code, but we ignore it.

$coan_exe = ".\build\_deps\coan-src\coan-6.0.1-x86_64.exe"
& "$coan_exe" source -D__cplusplus -UCINTERFACE ${Source} -V | Out-File $OutFile

if( $LASTEXITCODE -eq 0x11){
  Write-Host "Expected exit code 0x11. OK."
}else{
  Write-Host "Exit code $LASTEXITCODE"
  Exit 1
}