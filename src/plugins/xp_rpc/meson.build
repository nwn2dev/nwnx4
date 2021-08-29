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

xp_rpc_include_dirs = []
xp_rpc_include_dirs += include_dirs
xp_rpc_include_dirs += include_directories(['proto'])

shared_library('xp_rpc',
    [
        '../../misc/log.cpp',
        '../../HookHorror/proto/log.grpc.pb.cc',
        '../../HookHorror/proto/log.pb.cc',
        '../../HookHorror/LogConfig.cpp',
        '../../HookHorror/LogClient.cpp',
        '../plugin.cpp',
        'proto/NWScript/nwnx.grpc.pb.cc',
        'proto/NWScript/nwnx.pb.cc',
        'proto/NWScript/types.pb.cc',
        'RpcPlugin.cpp',
        'RpcClient.cpp'
    ],
    include_directories: xp_rpc_include_dirs,
    dependencies: [
        cppcompiler.find_library('wsock32'),
        cppcompiler.find_library('ws2_32'),
        cppcompiler.find_library('detours', dirs: lib_dirs),
    ] + extra_dep + grpc_dep,
    install: true
)

install_data('xp_rpc.yml', install_dir: '.')
