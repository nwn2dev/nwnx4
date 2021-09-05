if get_option('buildtype') == 'release'
    extra_dep = [
        cppcompiler.find_library('zlib', dirs: lib_dirs),
    ]
else
    extra_dep = [
        cppcompiler.find_library('zlibd', dirs: lib_dirs),
    ]
endif

extra_dep += [
    cppcompiler.find_library('libyaml-cppmdd', dirs: lib_dirs)
]

shared_library('xp_rpc',
    [
        '../../misc/log.cpp',
        '../plugin.cpp',
        'RpcPlugin.cpp',
        'RpcClient.cpp',
        '../../RPC/proto/NWScript/nwnx.grpc.pb.cc',
        '../../RPC/proto/NWScript/nwnx.pb.cc',
        '../../RPC/proto/NWScript/types.pb.cc',
        '../../RPC/proto/log.grpc.pb.cc',
        '../../RPC/proto/log.pb.cc',
        '../../RPC/LogConfig.cpp',
        '../../RPC/LogClient.cpp'
    ],
    include_directories: include_dirs,
    dependencies: [
        cppcompiler.find_library('wsock32'),
        cppcompiler.find_library('ws2_32'),
        cppcompiler.find_library('detours', dirs: lib_dirs),
    ] + extra_dep + grpc_dep,
    install: true
)

install_data('xp_rpc.yml', install_dir: '.')
