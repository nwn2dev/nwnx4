
NWNx4Controller_exe = executable('NWNX4_Controller',
    [
        'main.cpp',
        'controller.cpp',
        'service.cpp',
        'udp.cpp',
        '../misc/log.cpp',
    ],
    win_subsystem: 'console',
    cpp_args: ['/D_CONSOLE', '/D_CRT_SECURE_NO_DEPRECATE'],
    include_directories: include_dirs,
    dependencies: [
        cppcompiler.find_library('wsock32'),
        cppcompiler.find_library('ws2_32'),
        cppcompiler.find_library('detours', dirs: lib_dirs),
    ],
    install: true,
)

install_data('nwnx.ini', install_dir: '.')
install_data('nwnx_restart.cmd', install_dir: '.')
install_data('nwnx_include.nss', install_dir: 'nwscript')
