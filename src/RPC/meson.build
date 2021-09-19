# gRPC
if get_option('buildtype') == 'release'
    grpc_dep = [
        cppcompiler.find_library('libprotobuf', dirs: lib_dirs),
        cppcompiler.find_library('libprotoc', dirs: lib_dirs),
        cppcompiler.find_library('libprotobuf-lite', dirs: lib_dirs)
    ]
else
    grpc_dep = [
        cppcompiler.find_library('libprotobufd', dirs: lib_dirs),
        cppcompiler.find_library('libprotocd', dirs: lib_dirs),
        cppcompiler.find_library('libprotobuf-lited', dirs: lib_dirs)
    ]
endif

grpc_dep += [
    cppcompiler.find_library('gpr', dirs: lib_dirs),
    cppcompiler.find_library('grpc', dirs: lib_dirs),
    cppcompiler.find_library('grpc_csharp_ext', dirs: lib_dirs),
    cppcompiler.find_library('grpc_plugin_support', dirs: lib_dirs),
    cppcompiler.find_library('grpc_upbdefs', dirs: lib_dirs),
    cppcompiler.find_library('grpc++', dirs: lib_dirs),
    cppcompiler.find_library('grpc++_alts', dirs: lib_dirs),
    cppcompiler.find_library('grpc++', dirs: lib_dirs),
    cppcompiler.find_library('grpc++_error_details', dirs: lib_dirs),
    cppcompiler.find_library('address_sorting', dirs: lib_dirs),
    cppcompiler.find_library('cares', dirs: lib_dirs),
    cppcompiler.find_library('re2', dirs: lib_dirs),
    cppcompiler.find_library('libcrypto', dirs: lib_dirs),
    cppcompiler.find_library('crypt32'),
    cppcompiler.find_library('libssl', dirs: lib_dirs),
    cppcompiler.find_library('upb', dirs: lib_dirs),
    cppcompiler.find_library('upb_fastdecode', dirs: lib_dirs),
    cppcompiler.find_library('upb_handlers', dirs: lib_dirs),
    cppcompiler.find_library('upb_json', dirs: lib_dirs),
    cppcompiler.find_library('upb_pb', dirs: lib_dirs),
    cppcompiler.find_library('upb_reflection', dirs: lib_dirs),
    cppcompiler.find_library('upb_textformat', dirs: lib_dirs),
]

grpc_dep += [
    cppcompiler.find_library('absl_bad_any_cast_impl', dirs: lib_dirs),
    cppcompiler.find_library('absl_bad_optional_access', dirs: lib_dirs),
    cppcompiler.find_library('absl_bad_variant_access', dirs: lib_dirs),
    cppcompiler.find_library('absl_base', dirs: lib_dirs),
    cppcompiler.find_library('absl_city', dirs: lib_dirs),
    cppcompiler.find_library('absl_civil_time', dirs: lib_dirs),
    cppcompiler.find_library('absl_cord', dirs: lib_dirs),
    cppcompiler.find_library('absl_debugging_internal', dirs: lib_dirs),
    cppcompiler.find_library('absl_demangle_internal', dirs: lib_dirs),
    cppcompiler.find_library('absl_examine_stack', dirs: lib_dirs),
    cppcompiler.find_library('absl_exponential_biased', dirs: lib_dirs),
    cppcompiler.find_library('absl_failure_signal_handler', dirs: lib_dirs),
    cppcompiler.find_library('absl_flags', dirs: lib_dirs),
    cppcompiler.find_library('absl_flags_commandlineflag', dirs: lib_dirs),
    cppcompiler.find_library('absl_flags_commandlineflag_internal', dirs: lib_dirs),
    cppcompiler.find_library('absl_flags_config', dirs: lib_dirs),
    cppcompiler.find_library('absl_flags_internal', dirs: lib_dirs),
    cppcompiler.find_library('absl_flags_marshalling', dirs: lib_dirs),
    cppcompiler.find_library('absl_flags_parse', dirs: lib_dirs),
    cppcompiler.find_library('absl_flags_private_handle_accessor', dirs: lib_dirs),
    cppcompiler.find_library('absl_flags_program_name', dirs: lib_dirs),
    cppcompiler.find_library('absl_flags_reflection', dirs: lib_dirs),
    cppcompiler.find_library('absl_flags_usage', dirs: lib_dirs),
    cppcompiler.find_library('absl_flags_usage_internal', dirs: lib_dirs),
    cppcompiler.find_library('absl_graphcycles_internal', dirs: lib_dirs),
    cppcompiler.find_library('absl_hash', dirs: lib_dirs),
    cppcompiler.find_library('absl_hashtablez_sampler', dirs: lib_dirs),
    cppcompiler.find_library('absl_int128', dirs: lib_dirs),
    cppcompiler.find_library('absl_leak_check', dirs: lib_dirs),
    cppcompiler.find_library('absl_leak_check_disable', dirs: lib_dirs),
    cppcompiler.find_library('absl_log_severity', dirs: lib_dirs),
    cppcompiler.find_library('absl_malloc_internal', dirs: lib_dirs),
    cppcompiler.find_library('absl_periodic_sampler', dirs: lib_dirs),
    cppcompiler.find_library('absl_random_distributions', dirs: lib_dirs),
    cppcompiler.find_library('absl_random_internal_distribution_test_util', dirs: lib_dirs),
    cppcompiler.find_library('absl_random_internal_platform', dirs: lib_dirs),
    cppcompiler.find_library('absl_random_internal_pool_urbg', dirs: lib_dirs),
    cppcompiler.find_library('absl_random_internal_randen', dirs: lib_dirs),
    cppcompiler.find_library('absl_random_internal_randen_hwaes', dirs: lib_dirs),
    cppcompiler.find_library('absl_random_internal_randen_hwaes_impl', dirs: lib_dirs),
    cppcompiler.find_library('absl_random_internal_randen_slow', dirs: lib_dirs),
    cppcompiler.find_library('absl_random_internal_seed_material', dirs: lib_dirs),
    cppcompiler.find_library('absl_random_seed_gen_exception', dirs: lib_dirs),
    cppcompiler.find_library('absl_random_seed_sequences', dirs: lib_dirs),
    cppcompiler.find_library('absl_raw_hash_set', dirs: lib_dirs),
    cppcompiler.find_library('absl_raw_logging_internal', dirs: lib_dirs),
    cppcompiler.find_library('absl_scoped_set_env', dirs: lib_dirs),
    cppcompiler.find_library('absl_spinlock_wait', dirs: lib_dirs),
    cppcompiler.find_library('absl_stacktrace', dirs: lib_dirs),
    cppcompiler.find_library('absl_status', dirs: lib_dirs),
    cppcompiler.find_library('absl_statusor', dirs: lib_dirs),
    cppcompiler.find_library('absl_str_format_internal', dirs: lib_dirs),
    cppcompiler.find_library('absl_strerror', dirs: lib_dirs),
    cppcompiler.find_library('absl_strings', dirs: lib_dirs),
    cppcompiler.find_library('absl_strings_internal', dirs: lib_dirs),
    cppcompiler.find_library('absl_symbolize', dirs: lib_dirs),
    cppcompiler.find_library('absl_synchronization', dirs: lib_dirs),
    cppcompiler.find_library('absl_throw_delegate', dirs: lib_dirs),
    cppcompiler.find_library('absl_time', dirs: lib_dirs),
    cppcompiler.find_library('absl_time_zone', dirs: lib_dirs),
    cppcompiler.find_library('absl_wyhash', dirs: lib_dirs),
]

if get_option('buildtype') == 'release'
    extra_dep = [
        cppcompiler.find_library('zlib', dirs: lib_dirs),
    ]
else
    extra_dep = [
        cppcompiler.find_library('zlibd', dirs: lib_dirs),
    ]
endif

protoc = find_program('protoc', dirs: [nwnx4_repo_path + '/vcpkg_installed/x64-windows/tools/protobuf'])
grpc_cpp_plugin = find_program('grpc_cpp_plugin', dirs: [nwnx4_repo_path + '/vcpkg_installed/x64-windows/tools/grpc'])
grpc_gen = generator(protoc,
  output: ['@BASENAME@.grpc.pb.cc', '@BASENAME@.grpc.pb.h', '@BASENAME@.pb.cc', '@BASENAME@.pb.h'],
  arguments: ['-I=@CURRENT_SOURCE_DIR@', '--plugin=protoc-gen-grpc=' + grpc_cpp_plugin.path(), '--grpc_out=@BUILD_DIR@', '--cpp_out=@BUILD_DIR@', '@INPUT@']
)

_nwnx_rpc_gen = grpc_gen.process(
    [
        'proto/log.proto',
        'proto/NWScript/nwnx.proto',
        'proto/NWScript/types.proto',
    ],
    # This makes proto files being available under proto/<path> folder
    preserve_path_from: meson.current_source_dir(),
)

nwnx_rpc_dep = declare_dependency(
    sources: _nwnx_rpc_gen,
    dependencies: [
        cppcompiler.find_library('wsock32'),
        cppcompiler.find_library('ws2_32'),
    ] + extra_dep + grpc_dep,
    include_directories: ['.'],
)
