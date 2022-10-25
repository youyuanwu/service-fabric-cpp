param(
    [ValidateSet("Add","Remove","Connect", "Resolve", "Echo")]
    [String]
    $Action,
    [ValidateSet("Manual", "Test")]
    [String]
    $Mode = "Manual"
)
$ErrorActionPreference = "Stop";

$path = "build\kvstore_root"
$imageStorePath = "MyKvStoreAppV1x"

if($Action -eq "Connect"){
    Connect-ServiceFabricCluster
}elseif ($Action -eq "Add") {
    Connect-ServiceFabricCluster
    Test-ServiceFabricApplicationPackage -ApplicationPackagePath $path

    Copy-ServiceFabricApplicationPackage -ApplicationPackagePath $path -ApplicationPackagePathInImageStore $imageStorePath -TimeoutSec 1800
    
    Register-ServiceFabricApplicationType -ApplicationPathInImageStore $imageStorePath
    New-ServiceFabricApplication fabric:/KvStore KvStore 0.0.1
}elseif($Action -eq "Remove"){
    Connect-ServiceFabricCluster
    Remove-ServiceFabricApplication fabric:/KvStore -Force
    Unregister-ServiceFabricApplicationType KvStore 0.0.1 -Force
    Remove-ServiceFabricApplicationPackage -ApplicationPackagePathInImageStore $imageStorePath -Force
}elseif($Action -eq "Resolve"){
    Connect-ServiceFabricCluster
    Resolve-ServiceFabricService -ServiceName fabric:/KvStore/KvStoreService -PartitionKindSingleton -ForceRefresh
}