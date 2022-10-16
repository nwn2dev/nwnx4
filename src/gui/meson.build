wx_args = ['/DwxUSE_GUI=1', '/DwxUSE_UNICODE']
if get_option('buildtype') == 'release'
    wx_args += ['/D__WXMSWU__']
    wx_dep = [
        cppcompiler.find_library('jpeg', dirs: lib_dirs),
        cppcompiler.find_library('libpng16', dirs: lib_dirs),
        cppcompiler.find_library('wxbase32u', dirs: lib_dirs),
        cppcompiler.find_library('wxbase32u_net', dirs: lib_dirs),
        cppcompiler.find_library('wxmsw32u_core', dirs: lib_dirs),
        cppcompiler.find_library('zlib', dirs: lib_dirs),
    ]
else
    wx_args += ['/D__WXMSWUD__', '/D__WXDEBUG__']
    wx_dep = [
        cppcompiler.find_library('jpeg', dirs: lib_dirs),
        cppcompiler.find_library('libpng16d', dirs: lib_dirs),
        cppcompiler.find_library('wxbase32ud', dirs: lib_dirs),
        cppcompiler.find_library('wxbase32ud_net', dirs: lib_dirs),
        cppcompiler.find_library('wxmsw32ud_core', dirs: lib_dirs),
        cppcompiler.find_library('zlibd', dirs: lib_dirs),
    ]
endif

NWNx4Gui_exe = executable('NWNX4_Gui',
    [
        'mainframe.cpp',
        '../controller/controller.cpp',
        '../controller/udp.cpp',
        'dlgabout.cpp',
        'nwnx.cpp',
        'worker.cpp',
        'main.cpp',
        '../misc/log.cpp',
        'GuiLog.cpp'
    ],
    nwnx_version,
    cpp_pch : 'pch/pch.h',

    win_subsystem: 'windows',
    cpp_args: wx_args,
    include_directories: include_dirs,
    dependencies: [
        cppcompiler.find_library('ws2_32'),
        cppcompiler.find_library('wsock32'),
        cppcompiler.find_library('comctl32'),
        cppcompiler.find_library('rpcrt4'),
        cppcompiler.find_library('detours', dirs: lib_dirs)
    ] + wx_dep,
    implib: false,
    install: true,
)
