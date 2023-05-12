# service fabric cpp
![ci](https://github.com/youyuanwu/service-fabric-cpp/actions/workflows/build.yaml/badge.svg)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://raw.githubusercontent.com/youyuanwu/service-fabric-cpp/main/LICENSE)

Service Fabric C++ Community SDK.

Service Fabric is open sourced on github: `https://github.com/microsoft/service-fabric`.
The latest open sourced version of SF is `6.4`.
This SDK only provides `6.4` functionalities. New functionalities in newer versions is are not accessible in this SDK.

The Fabric runtime and client are accessible from the dlls installed from service fabric runtime.
The c headers are generated from open sourced idls in repo [service-fabric](https://github.com/microsoft/service-fabric/tree/master/src/prod/src/idl/public)

This lib is developed for educational purposes, and not intended for production.
Linux is not supported.

## Dependencies
* Install service fabric runtime. See [get-started](https://learn.microsoft.com/en-us/azure/service-fabric/service-fabric-get-started)
* Install Boost. Boost libs used: Boost asio, Boost log.
* Install Visual Studio msvc tool chain.
* CMake will also auto download dependencies `magic_enum` and `moderncom`.

## Build
```
cmake . -B build
cmake --build build
```

## Example App
In `Examples/echoapp` is a service fabric reliable stateless singleton app. It runs an boost asio tcp echo server.
After build, the artifact in `build\echoapp_root` can be deployed to SF clusters.

## Test
When building this repo on an machine with service fabric developer cluster started,
```ps1
.\tests\echo_script_test.ps1
```
Creates Echo app in the cluster, and makes an tcp echo call, then finally removes the app.

## License
MIT License