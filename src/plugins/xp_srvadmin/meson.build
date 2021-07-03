
shared_library('xp_srvadmin',
    [
        '../../misc/log.cpp',
        '../plugin.cpp',
        'srvadmin.cpp',
    ],
    cpp_args: ['/D_WIN32_WINNT=0x0400'],
    link_args: ['/dynamicbase', '/nxcompat'],
    include_directories: include_dirs,
    install: true,
)

install_data('nwnx_srvadmin.nss', install_dir: 'nwscript')
