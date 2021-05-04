project(hhHook VERSION 1.0)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DWIN32 /D_DEBUG /D_WINDOWS /D_USRDLL /DTEST_EXPORTS /D__WXMSWUD__ /D__WXDEBUG__")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /DWIN32 /DNDEBUG /D_CONSOLE /D_WINDOWS /D_USRDLL /DTEST_EXPORTS /D__WXMSWU__")
set(CMAKE_EXE_LINKER_FLAGS "/NODEFAULTLIB:MSVCRT /ASSEMBLYDEBUG")
set(SOURCE_FILES hook.cpp crashdump.cpp ../plugins/legacy_plugin.cpp ../plugins/plugin.cpp ../misc/log.cpp ../plugins/plugin.cpp stdwx.cpp hook.map)
set(VCPKG_INCLUDE_DIR C:/opt/vcpkg/installed/x86-windows-static-md/include/)
set(VCPKG_LIB_DIR_DEBUG C:/opt/vcpkg/installed/x86-windows-static-md/debug/lib/)
set(VCPKG_LIB_DIR_RELEASE C:/opt/vcpkg/installed/x86-windows-static-md/lib/)

add_library(hhHook SHARED ${SOURCE_FILES})

set_source_files_properties(hook.map PROPERTIES HEADER_FILE_ONLY TRUE)
target_precompile_headers(hhHook PUBLIC stdwx.h)
target_include_directories(hhHook PRIVATE
        ../controller ../misc ../../lib/NWN2/include ${VCPKG_INCLUDE_DIR} ${VCPKG_LIB_DIR_DEBUG}/mswud ${VCPKG_LIB_DIR_RELEASE}/mswu)
target_link_directories(hhHook PRIVATE
        ../lib/NWN2 ${VCPKG_LIB_DIR_DEBUG} ${VCPKG_LIB_DIR_RELEASE})
target_link_libraries(hhHook PRIVATE
        comctl32.lib  rpcrt4.lib ws2_32.lib wsock32.lib
        detours.lib
        wxmsw31ud_core.lib wxbase31ud_net.lib wxbase31ud.lib wxregexud.lib)
