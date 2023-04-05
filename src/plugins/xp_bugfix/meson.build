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
    cpp_args: ['/permissive'],
    link_args: ['/dynamicbase', '/nxcompat'],
    vs_module_defs: 'xp_bugfix.def',
    include_directories: include_dirs,
    dependencies: [
        cppcompiler.find_library('dbghelp'),
        cppcompiler.find_library('ws2_32'),
        cppcompiler.find_library('cryptopp', dirs: lib_dirs)
    ],
    install: true,
    install_dir: 'plugins',
)

install_data('xp_bugfix.ini', install_dir: 'config.example')
install_data('nwnx_bugfix.nss', install_dir: 'nwscript')

install_data(meson.global_source_root() + '/lib/auroraserverdll/lib/AuroraServerMsgCheck.dll', install_dir: 'nwn2server-dll')
install_data(meson.global_source_root() + '/lib/auroraserverdll/lib/AuroraServerMsgCheck.pdb', install_dir: 'nwn2server-dll')
install_data(meson.global_source_root() + '/lib/auroraserverdll/lib/AuroraServerNetLayer.dll', install_dir: 'nwn2server-dll')
install_data(meson.global_source_root() + '/lib/auroraserverdll/lib/AuroraServerNetLayer.pdb', install_dir: 'nwn2server-dll')
