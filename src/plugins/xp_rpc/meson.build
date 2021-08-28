yaml_dep = [
    cppcompiler.find_library('libyaml-cppmdd', dirs: lib_dirs)
]

shared_library('xp_rpc',
    [
        '../../misc/log.cpp',
        '../../misc/grpc/LogClient.cpp',
        '../../misc/grpc/proto/log.grpc.pb.cc',
        '../../misc/grpc/proto/log.pb.cc',

        '../plugin.cpp',
        'RpcPlugin.cpp'
    ],
    cpp_args: ['/D_WIN32_WINNT=0x0400'],
    link_args: ['/dynamicbase', '/nxcompat'],
    include_directories: include_dirs,
    dependencies: grpc_dep + yaml_dep,
    install: true
)
