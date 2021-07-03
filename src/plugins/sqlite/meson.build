

shared_library('xp_sqlite',
    [
    	'../../misc/log.cpp',
    	'../dbplugin.cpp',
    	'../plugin.cpp',
    	'lib/sqlite3.c',
    	'sqlite.cpp',
    ],
    include_directories: include_dirs,
    install: true,
)

install_data('xp_sqlite.ini', install_dir: '.')
install_data('../mysql/nwnx_sql.nss', install_dir: 'nwscript')
