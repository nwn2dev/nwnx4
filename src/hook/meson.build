

NWNX4_Hook_lib = shared_library('NWNX4_Hook',
    [
        'hook.cpp',
        'crashdump.cpp',
        '../plugins/legacy_plugin.cpp',
        '../plugins/plugin.cpp',
        '../misc/log.cpp',
    ],
    include_directories: include_dirs,
    dependencies: [cppcompiler.find_library('detours', dirs: lib_dirs)],
    install: true,
)
