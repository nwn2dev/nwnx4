

NWNX4_Hook_lib = shared_library('NWNX4_Hook',
    [
        'hook.cpp',
        'crashdump.cpp',
        '../plugins/CPlugin.cpp',
        '../plugins/CPluginVersion.cpp',
        '../plugins/plugin.cpp',
        '../plugins/legacy_plugin.cpp',
        '../misc/log.cpp',
    ],
    include_directories: include_dirs,
    dependencies: [cppcompiler.find_library('detours', dirs: lib_dirs)],
    install: true,
)
