source_files = ['stdwx.cpp', 'mainframe.cpp', '../controller/controller.cpp', '../controller/udp.cpp', 'dlgabout.cpp', 'nwnx.cpp', 'worker.cpp', 'main.cpp', '../misc/log.cpp', 'GuiLog.cpp', 'LogServiceImpl.cpp', 'log.grpc.pb.cc', 'log.pb.cc']
libs = [
    cpp.find_library('comctl32'), cpp.find_library('rpcrt4'), cpp.find_library('ws2_32'),
    cpp.find_library('wsock32'), cpp.find_library('crypt32'),

    cpp.find_library('detours', dirs: lib_dirs),

    cpp.find_library('gpr', dirs: lib_dirs), cpp.find_library('grpc', dirs: lib_dirs), cpp.find_library('grpc_csharp_ext', dirs: lib_dirs),
    cpp.find_library('grpc_plugin_support', dirs: lib_dirs), cpp.find_library('grpc_upbdefs', dirs: lib_dirs), cpp.find_library('grpc++', dirs: lib_dirs),
    cpp.find_library('grpc++_alts', dirs: lib_dirs), cpp.find_library('grpc++', dirs: lib_dirs), cpp.find_library('grpc++_error_details', dirs: lib_dirs),
    cpp.find_library('address_sorting', dirs: lib_dirs), cpp.find_library('cares', dirs: lib_dirs), cpp.find_library('re2', dirs: lib_dirs),

    cpp.find_library('libcrypto', dirs: lib_dirs), cpp.find_library('libssl', dirs: lib_dirs), cpp.find_library('upb', dirs: lib_dirs),
    cpp.find_library('upb_fastdecode', dirs: lib_dirs), cpp.find_library('upb_handlers', dirs: lib_dirs), cpp.find_library('upb_json', dirs: lib_dirs),
    cpp.find_library('upb_pb', dirs: lib_dirs), cpp.find_library('upb_reflection', dirs: lib_dirs), cpp.find_library('upb_textformat', dirs: lib_dirs)]

if get_option('buildtype') == 'debug'
    cpp_args = ['/DWIN32', '/D_DEBUG', '/D__WXMSWUD__', '/D__WXDEBUG__', '/D_WINDOWS', '/DNOPCH', '/DwxUSE_GUI=1', '/DUNICODE', '/DwxUSE_UNICODE']
    libs += [
        cpp.find_library('wxmsw31ud_core', dirs: lib_dirs), cpp.find_library('wxbase31ud_net', dirs: lib_dirs), cpp.find_library('wxbase31ud', dirs: lib_dirs),

        cpp.find_library('jpegd', dirs: lib_dirs), cpp.find_library('libpng16d', dirs: lib_dirs), cpp.find_library('zlibd', dirs: lib_dirs),

        cpp.find_library('libprotobufd', dirs: lib_dirs), cpp.find_library('libprotocd', dirs: lib_dirs), cpp.find_library('libprotobuf-lited', dirs: lib_dirs)]
else
    cpp_args = ['/DWIN32', '/DNDEBUG', '/D__WXMSWU__', '/D_WINDOWS', '/DNOPCH', '/DwxUSE_GUI=1', '/DUNICODE', '/DwxUSE_UNICODE']
    libs = [
        cpp.find_library('wxmsw31u_core', dirs: lib_dirs), cpp.find_library('wxbase31u_net', dirs: lib_dirs), cpp.find_library('wxbase31u', dirs: lib_dirs),

        cpp.find_library('jpeg', dirs: lib_dirs), cpp.find_library('libpng16', dirs: lib_dirs), cpp.find_library('zlib', dirs: lib_dirs),

        cpp.find_library('libprotobuf', dirs: lib_dirs), cpp.find_library('libprotoc', dirs: lib_dirs), cpp.find_library('libprotobuf-lite', dirs: lib_dirs)]
endif

libs += [
  cpp.find_library('absl_bad_any_cast_impl', dirs: lib_dirs),
  cpp.find_library('absl_bad_optional_access', dirs: lib_dirs),
  cpp.find_library('absl_bad_variant_access', dirs: lib_dirs),
  cpp.find_library('absl_base', dirs: lib_dirs),
  cpp.find_library('absl_city', dirs: lib_dirs),
  cpp.find_library('absl_civil_time', dirs: lib_dirs),
  cpp.find_library('absl_cord', dirs: lib_dirs),
  cpp.find_library('absl_debugging_internal', dirs: lib_dirs),
  cpp.find_library('absl_demangle_internal', dirs: lib_dirs),
  cpp.find_library('absl_examine_stack', dirs: lib_dirs),
  cpp.find_library('absl_exponential_biased', dirs: lib_dirs),
  cpp.find_library('absl_failure_signal_handler', dirs: lib_dirs),
  cpp.find_library('absl_flags', dirs: lib_dirs),
  cpp.find_library('absl_flags_commandlineflag', dirs: lib_dirs),
  cpp.find_library('absl_flags_commandlineflag_internal', dirs: lib_dirs),
  cpp.find_library('absl_flags_config', dirs: lib_dirs),
  cpp.find_library('absl_flags_internal', dirs: lib_dirs),
  cpp.find_library('absl_flags_marshalling', dirs: lib_dirs),
  cpp.find_library('absl_flags_parse', dirs: lib_dirs),
  cpp.find_library('absl_flags_private_handle_accessor', dirs: lib_dirs),
  cpp.find_library('absl_flags_program_name', dirs: lib_dirs),
  cpp.find_library('absl_flags_reflection', dirs: lib_dirs),
  cpp.find_library('absl_flags_usage', dirs: lib_dirs),
  cpp.find_library('absl_flags_usage_internal', dirs: lib_dirs),
  cpp.find_library('absl_graphcycles_internal', dirs: lib_dirs),
  cpp.find_library('absl_hash', dirs: lib_dirs),
  cpp.find_library('absl_hashtablez_sampler', dirs: lib_dirs),
  cpp.find_library('absl_int128', dirs: lib_dirs),
  cpp.find_library('absl_leak_check', dirs: lib_dirs),
  cpp.find_library('absl_leak_check_disable', dirs: lib_dirs),
  cpp.find_library('absl_log_severity', dirs: lib_dirs),
  cpp.find_library('absl_malloc_internal', dirs: lib_dirs),
  cpp.find_library('absl_periodic_sampler', dirs: lib_dirs),
  cpp.find_library('absl_random_distributions', dirs: lib_dirs),
  cpp.find_library('absl_random_internal_distribution_test_util', dirs: lib_dirs),
  cpp.find_library('absl_random_internal_platform', dirs: lib_dirs),
  cpp.find_library('absl_random_internal_pool_urbg', dirs: lib_dirs),
  cpp.find_library('absl_random_internal_randen', dirs: lib_dirs),
  cpp.find_library('absl_random_internal_randen_hwaes', dirs: lib_dirs),
  cpp.find_library('absl_random_internal_randen_hwaes_impl', dirs: lib_dirs),
  cpp.find_library('absl_random_internal_randen_slow', dirs: lib_dirs),
  cpp.find_library('absl_random_internal_seed_material', dirs: lib_dirs),
  cpp.find_library('absl_random_seed_gen_exception', dirs: lib_dirs),
  cpp.find_library('absl_random_seed_sequences', dirs: lib_dirs),
  cpp.find_library('absl_raw_hash_set', dirs: lib_dirs),
  cpp.find_library('absl_raw_logging_internal', dirs: lib_dirs),
  cpp.find_library('absl_scoped_set_env', dirs: lib_dirs),
  cpp.find_library('absl_spinlock_wait', dirs: lib_dirs),
  cpp.find_library('absl_stacktrace', dirs: lib_dirs),
  cpp.find_library('absl_status', dirs: lib_dirs),
  cpp.find_library('absl_statusor', dirs: lib_dirs),
  cpp.find_library('absl_str_format_internal', dirs: lib_dirs),
  cpp.find_library('absl_strerror', dirs: lib_dirs),
  cpp.find_library('absl_strings', dirs: lib_dirs),
  cpp.find_library('absl_strings_internal', dirs: lib_dirs),
  cpp.find_library('absl_symbolize', dirs: lib_dirs),
  cpp.find_library('absl_synchronization', dirs: lib_dirs),
  cpp.find_library('absl_throw_delegate', dirs: lib_dirs),
  cpp.find_library('absl_time', dirs: lib_dirs),
  cpp.find_library('absl_time_zone', dirs: lib_dirs),
  cpp.find_library('absl_wyhash', dirs: lib_dirs)]

executable('NWNx4Gui', source_files,
    cpp_args: cpp_args, include_directories: include_directories(['../controller', '../misc', include_dirs]), dependencies: libs)
