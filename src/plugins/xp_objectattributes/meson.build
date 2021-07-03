
shared_library('xp_objectattributes',
    [
        '../../misc/log.cpp',
        '../plugin.cpp',
        'PluginCore.cpp',
        'xp_objectattributes.cpp',
    ],
    link_args: ['/dynamicbase', '/nxcompat'],
    include_directories: include_dirs,
    install: true,
)

install_data('nwnx_objectattributes.nss', install_dir: 'nwscript')
install_data('gui_test_objectattributes.nss', install_dir: 'unittest')
