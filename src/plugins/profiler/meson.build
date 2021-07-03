
shared_library('xp_profiler',
    [
		'../../misc/log.cpp',
        '../plugin.cpp',
        'hash.cpp',
        'hook.cpp',
        'profiler.cpp',
    ],
    include_directories: include_dirs,
    dependencies: [
        cppcompiler.find_library('detours', dirs: lib_dirs),
    ],
    install: true,
)
