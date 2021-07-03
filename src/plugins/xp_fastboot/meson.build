
shared_library('xp_fastboot',
    [
        '../../misc/IATHook.cpp',
        '../../misc/log.cpp',
        '../plugin.cpp',
        'fastboot.cpp',
    ],
    cpp_args: ['/D_WIN32_WINNT=0x0500'],
    link_args: ['/dynamicbase', '/nxcompat'],
    include_directories: include_dirs,
    install: true,
)
