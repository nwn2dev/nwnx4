

NWNX4_Hook_lib = shared_library('NWNX4_Hook',
    [
        'hook.cpp',
        'scorcohook.cpp',
        'crashdump.cpp',
        'CPlugin.cpp',
        '../plugins/plugin.cpp',
        '../plugins/legacy_plugin.cpp',
        '../misc/log.cpp',
    ],
    include_directories: include_dirs,
    dependencies: [
        cppcompiler.find_library('detours', dirs: lib_dirs),
        cppcompiler.find_library('NWN2_MemoryMgr_amdxp', dirs: lib_dirs),
    ],
    install: true,
)
