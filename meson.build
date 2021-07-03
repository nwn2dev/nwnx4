project(
    'NWNX4',
    ['cpp', 'c'],
    default_options : [
        # 'default_library=static',
        'cpp_std=c++latest',
        'bindir=.',
        'libdir=.',
        'datadir=.',
    ],
)

# Global compiler arguments
cppcompiler = meson.get_compiler('cpp')
add_project_arguments('/permissive', '/DWIN32', '/D_WINDOWS', '/D_USRDLL', '/DTEST_EXPORTS', '/D_UNICODE', '/DUNICODE', language: ['cpp', 'c'])
if get_option('buildtype') == 'release'
    add_project_arguments('/DNDEBUG', language: ['cpp', 'c'])
    # add_global_link_arguments('/GL', language: ['cpp', 'c'])
else
    add_project_arguments('/D_DEBUG', language: ['cpp', 'c'])
endif

# Include & lib paths
nwnx4_repo_path = meson.global_source_root()
include_dirs = include_directories([
    'lib/NWN2/include',
    'vcpkg_installed/x86-windows-static-md/include',
])
lib_dirs = [
    nwnx4_repo_path + '/lib/NWN2',
    nwnx4_repo_path + '/vcpkg_installed/x86-windows-static-md/lib',
]
if get_option('buildtype') == 'release'
    lib_dirs += nwnx4_repo_path + '/vcpkg_installed/x86-windows-static-md/lib'
else
    lib_dirs += nwnx4_repo_path + '/vcpkg_installed/x86-windows-static-md/debug/lib'
endif


# Target list
subdir('src/hook')
subdir('src/controller')
subdir('src/gui')

nwscript_includes = []

# subdir('src/plugins/chat') # Obsolete ?
subdir('src/plugins/funcs')
# subdir('src/plugins/leto') # Obsolete / not ported
subdir('src/plugins/mysql')
subdir('src/plugins/pickpocket')
subdir('src/plugins/profiler')
# subdir('src/plugins/ruby') # Not ported
subdir('src/plugins/sqlite')
subdir('src/plugins/time')
# subdir('src/plugins/vaultster') # TODO
subdir('src/plugins/xp_bugfix')
subdir('src/plugins/xp_fastboot')
subdir('src/plugins/xp_objectattributes')
subdir('src/plugins/xp_srvadmin')
