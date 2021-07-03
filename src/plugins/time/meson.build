
shared_library('xp_time',
    [
        '../plugin.cpp',
        '../../misc/log.cpp',
        'time_plugin.cpp',
    ],
    include_directories: include_dirs,
    dependencies: [
        cppcompiler.find_library('detours', dirs: lib_dirs),
    ],
    install: true,
)

install_data('nwnx_time.nss', install_dir: 'nwscript')
