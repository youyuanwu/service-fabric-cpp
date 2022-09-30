# This is not used since github action has service fabric installed

$ErrorActionPreference = "Stop";
Set-PSDebug -Trace 1

$url = "https://download.microsoft.com/download/b/8/a/b8a2fb98-0ec1-41e5-be98-9d8b5abf7856/MicrosoftServiceFabric.9.0.1107.9590.exe"

$path = ".\build"   
If (!(test-path $path))
{
  New-Item $path -ItemType Directory
}

Invoke-WebRequest $url -OutFile .\build\MicrosoftServiceFabric.9.0.1107.9590.exe

.\build\MicrosoftServiceFabric.9.0.1107.9590.exe /accepteula