# HookHorror
.\vcpkg_installed\x86-windows-static-md\tools\protobuf\protoc.exe `
    -I .\lib\proto `
    --grpc_out=.\src\HookHorror\proto --cpp_out=.\src\HookHorror\proto `
    --plugin=protoc-gen-grpc=.\vcpkg_installed\x64-windows\tools\grpc\grpc_cpp_plugin.exe `
    .\lib\proto\log.proto

# xp_rpc
.\vcpkg_installed\x86-windows-static-md\tools\protobuf\protoc.exe `
    -I .\lib\proto `
    --grpc_out=.\src\plugins\xp_rpc\proto --cpp_out=.\src\plugins\xp_rpc\proto `
    --plugin=protoc-gen-grpc=.\vcpkg_installed\x64-windows\tools\grpc\grpc_cpp_plugin.exe `
    .\lib\proto\NWScript\nwnx.proto

.\vcpkg_installed\x86-windows-static-md\tools\protobuf\protoc.exe `
    -I .\lib\proto `
    --cpp_out=.\src\plugins\xp_rpc\proto `
    .\lib\proto\NWScript\types.proto
