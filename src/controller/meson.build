if get_option('buildtype') == 'release'
    extra_dep = [
        cppcompiler.find_library('zlib', dirs: lib_dirs),
    ]
else
    extra_dep = [
        cppcompiler.find_library('zlibd', dirs: lib_dirs),
    ]
endif

NWNx4Controller_exe = executable('NWNX4_Controller',
    [
        'main.cpp',
        'controller.cpp',
        'service.cpp',
        'udp.cpp',
        '../misc/log.cpp',
        'RPC/Service.cpp',
        'RPC/LogService.cpp',
        'RPC/LogServiceImpl.cpp',
        '../RPC/LogConfig.cpp',
    ],
    nwnx_version,

    win_subsystem: 'console',
    cpp_args: ['/D_CONSOLE', '/D_CRT_SECURE_NO_DEPRECATE'],
    include_directories: include_dirs,
    dependencies: [
        cppcompiler.find_library('wsock32'),
        cppcompiler.find_library('ws2_32'),
        cppcompiler.find_library('detours', dirs: lib_dirs),
        nwnx_rpc_dep,
    ],
    install: true,
)

install_data('nwnx.ini', install_dir: 'config.example')
install_data('nwnx_restart.cmd', install_dir: 'config.example')
install_data('nwnx_include.nss', install_dir: 'nwscript')
