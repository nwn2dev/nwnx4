# xp_funcs = shared_library('xp_funcs',
# 	'../plugin.cpp',
# 	'../../misc/log.cpp',
# 	'funcs.cpp',
# 	include_directories: ['.'],
# 	# dependencies: [],
# 	# include_directories: ['../External/CRTCompat/inc'],
# 	# cpp_pch : './Precomp.h'
# )



shared_library('xp_funcs',
    [
		'../plugin.cpp',
		'../../misc/log.cpp',
		'funcs.cpp',
    ],
    include_directories: include_dirs,
    install: true,
)

install_data('nwnx_funcs.nss', install_dir: 'nwscript')
