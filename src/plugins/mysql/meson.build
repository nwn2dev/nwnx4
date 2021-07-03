

shared_library('xp_mysql',
    [
    	'../../misc/log.cpp',
		'../dbplugin.cpp',
		'../plugin.cpp',
		'../scorcohook.cpp',
		'mysql_plugin.cpp',
    ],
    include_directories: include_dirs,
    dependencies: [
    	# MariaDB & dependencies
        cppcompiler.find_library('libmariadb', dirs: lib_dirs),
    	cppcompiler.find_library('mariadbclient', dirs: lib_dirs),
        cppcompiler.find_library('zlib', dirs: lib_dirs),
        cppcompiler.find_library('libssl', dirs: lib_dirs),
        cppcompiler.find_library('libcrypto', dirs: lib_dirs),
        cppcompiler.find_library('shlwapi'),
        cppcompiler.find_library('crypt32'),

    	cppcompiler.find_library('detours', dirs: lib_dirs),
    	cppcompiler.find_library('NWN2_MemoryMgr_amdxp', dirs: lib_dirs),
        cppcompiler.find_library('dbghelp'),
        cppcompiler.find_library('ws2_32'),
    ],
    install: true,
)

install_data('xp_mysql.ini', install_dir: '.')
install_data('nwnx_sql.nss', install_dir: 'nwscript')
install_data('nwnx_mysql_errors.nss', install_dir: 'nwscript')
