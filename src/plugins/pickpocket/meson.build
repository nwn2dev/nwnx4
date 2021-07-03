
shared_library('xp_pickpocket',
    [
        '../plugin.cpp',
        '../../misc/log.cpp',
        'Pickpocket.cpp',
    ],
    include_directories: include_dirs,
    dependencies: [
        cppcompiler.find_library('detours', dirs: lib_dirs),
    ],
    install: true,
)

install_data('xp_pickpocket.ini', install_dir: '.')
install_data('nwnx_pickpocket.nss', install_dir: 'nwscript')
