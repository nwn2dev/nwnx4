if get_option('buildtype') == 'release'
    extra_dep = [
        cppcompiler.find_library('zlib', dirs: lib_dirs),
        cppcompiler.find_library('libyaml-cppmd', dirs: lib_dirs),
    ]
else
    extra_dep = [
        cppcompiler.find_library('zlibd', dirs: lib_dirs),
        cppcompiler.find_library('libyaml-cppmdd', dirs: lib_dirs),
    ]
endif

shared_library('xp_rpc',
    [
        '../../misc/log.cpp',
        '../plugin.cpp',
        'RpcPlugin.cpp',
        'RpcClient.cpp',
        '../../RPC/LogConfig.cpp',
        '../../RPC/LogClient.cpp'
    ],
    include_directories: include_dirs,
    dependencies: [
        cppcompiler.find_library('wsock32'),
        cppcompiler.find_library('ws2_32'),
        cppcompiler.find_library('detours', dirs: lib_dirs),
        nwnx_rpc_dep,
    ] + extra_dep,
    install: true
)

install_data('xp_rpc.yml', install_dir: '.')
