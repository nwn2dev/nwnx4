

shared_library('xp_sqlite',
    [
    	'../../misc/log.cpp',
    	'../database/dbplugin.cpp',
    	'../plugin.cpp',
    	'lib/sqlite3.c',
    	'sqlite_plugin.cpp',
    ],
    include_directories: include_dirs,
    dependencies: [
        cppcompiler.find_library('NWN2_MemoryMgr_amdxp', dirs: lib_dirs),
        cppcompiler.find_library('detours', dirs: lib_dirs),
    ] + extra_libs,
    install: true,
    install_dir: 'plugins',
)

install_data('xp_sqlite.ini', install_dir: 'config.example')
install_data('../database/nwnx_sql.nss', install_dir: 'nwscript')
