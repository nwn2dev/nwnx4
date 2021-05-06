project(hhHook VERSION 1.0)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DWIN32 /D_DEBUG /D_WINDOWS /D_USRDLL /DTEST_EXPORTS /D__WXMSWUD__ /D__WXDEBUG__ /D_UNICODE /DwxUSE_UNICODE")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /DWIN32 /DNDEBUG /D_CONSOLE /D_WINDOWS /D_USRDLL /DTEST_EXPORTS /D__WXMSWU__ /D_UNICODE /DwxUSE_UNICODE")
set(CMAKE_EXE_LINKER_FLAGS "/NODEFAULTLIB:MSVCRT")
set(SOURCE_FILES hook.cpp crashdump.cpp ../plugins/legacy_plugin.cpp ../plugins/plugin.cpp ../misc/log.cpp ../plugins/plugin.cpp stdwx.cpp hook.map)

add_library(hhHook SHARED ${SOURCE_FILES})

set_source_files_properties(hook.map PROPERTIES HEADER_FILE_ONLY TRUE)
target_precompile_headers(hhHook PUBLIC stdwx.h)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_include_directories(hhHook PRIVATE ../controller ../misc ../../lib/NWN2/include ${VCPKG_INCLUDE_DIR} ${VCPKG_LIB_DIR_DEBUG}/mswud)
    target_link_directories(hhHook PRIVATE ../lib/NWN2 ${VCPKG_LIB_DIR_DEBUG})
    target_link_libraries(hhHook PRIVATE comctl32.lib rpcrt4.lib ws2_32.lib wsock32.lib fmtd.lib detours.lib wxmsw31ud_core.lib wxbase31ud_net.lib wxbase31ud.lib wxregexud.lib)
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_include_directories(hhHook PRIVATE ../controller ../misc ../../lib/NWN2/include ${VCPKG_INCLUDE_DIR} ${VCPKG_LIB_DIR_RELEASE}/mswu)
    target_link_directories(hhHook PRIVATE ../lib/NWN2 ${VCPKG_LIB_DIR_RELEASE})
    target_link_libraries(hhHook PRIVATE comctl32.lib rpcrt4.lib ws2_32.lib wsock32.lib fmt.lib detours.lib wxmsw31u_core.lib wxbase31u_net.lib wxbase31u.lib wxregexu.lib)
endif()
