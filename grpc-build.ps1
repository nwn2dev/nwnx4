# Controller
.\vcpkg_installed\x86-windows-static-md\tools\protobuf\protoc.exe `
    -I .\lib\proto\util `
    --grpc_out=.\src\controller\proto --cpp_out=.\src\controller\proto `
    --plugin=protoc-gen-grpc=.\vcpkg_installed\x64-windows\tools\grpc\grpc_cpp_plugin.exe `
    .\lib\proto\util\log.proto
