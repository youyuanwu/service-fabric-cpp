param(
  [string] $Mode = "Debug"
)
$ErrorActionPreference = "Stop";

Write-Host "Using Mode $Mode"

$example_dir = ".\build\examples\fabricclient\$Mode"

Set-PSDebug -Trace 1

& "$example_dir\wait_main.exe"

& "$example_dir\asio_main.exe"

& "$example_dir\compose_main.exe"

Set-PSDebug -Off