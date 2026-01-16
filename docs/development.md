## Deploy app

```ps1
Connect-ServiceFabricCluster

$path = "build\echoapp_root"

Test-ServiceFabricApplicationPackage -ApplicationPackagePath build\echoapp_root

Copy-ServiceFabricApplicationPackage -ApplicationPackagePath $path -ApplicationPackagePathInImageStore MyApplicationV1 -TimeoutSec 1800

Register-ServiceFabricApplicationType -ApplicationPathInImageStore MyApplicationV1

  Get-ServiceFabricApplicationType

New-ServiceFabricApplication fabric:/EchoApp EchoApp 0.0.1
  Get-ServiceFabricApplication

Remove-ServiceFabricApplication fabric:/EchoApp
Unregister-ServiceFabricApplicationType EchoApp 0.0.1
Remove-ServiceFabricApplicationPackage -ApplicationPackagePathInImageStore MyApplicationV1


```

# TODO:
Migrate to modules after this vscode issue: https://github.com/microsoft/vscode-cpptools/issues/6302