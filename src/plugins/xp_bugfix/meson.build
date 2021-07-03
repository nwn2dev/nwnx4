
shared_library('xp_bugfix',
    [
        '../../misc/IATHook.cpp',
        '../../misc/log.cpp',
        '../../misc/Patch.cpp',
        '../plugin.cpp',
        'bugfix.cpp',
        'MD5.cpp',
        'NetLayer.cpp',
        'Reflector.cpp',
        'StackTracer.cpp',
    ],
    link_args: ['/dynamicbase', '/nxcompat'],
    include_directories: include_dirs,
    dependencies: [
        cppcompiler.find_library('dbghelp'),
        cppcompiler.find_library('ws2_32'),
    ],
    install: true,
)

install_data('xp_bugfix.ini', install_dir: '.')
install_data('nwnx_bugfix.nss', install_dir: 'nwscript')
