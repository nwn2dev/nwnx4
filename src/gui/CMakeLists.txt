project(hhGui VERSION 1.0)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DWIN32 /D_DEBUG /D__WXMSWUD__ /D__WXDEBUG__ /D_WINDOWS /DNOPCH /DwxUSE_GUI=1")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /DWIN32 /DNDEBUG /D__WXMSWU__ /D_WINDOWS /DNOPCH /DwxUSE_GUI=1")
set(CMAKE_EXE_LINKER_FLAGS "/NODEFAULTLIB:MSVCRT")
set(SOURCE_FILES WIN32 mainframe.cpp ../controller/controller.cpp ../controller/udp.cpp dlgabout.cpp nwnx.cpp stdwx.cpp worker.cpp main.cpp main.h)
set(VCPKG_INCLUDE_DIR C:/opt/vcpkg/installed/x86-windows-static-md/include/)
set(VCPKG_LIB_DIR_DEBUG C:/opt/vcpkg/installed/x86-windows-static-md/debug/lib/)
set(VCPKG_LIB_DIR_RELEASE C:/opt/vcpkg/installed/x86-windows-static-md/lib/)

add_executable(hhGui ${SOURCE_FILES})
install(TARGETS hhHook
        RUNTIME DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        LIBRARY DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

target_sources(hhGui PRIVATE "NWNX GUI.rc" nwnx.rc)
target_precompile_headers(hhGui PUBLIC stdwx.h)
target_include_directories(hhGui PRIVATE
        ../controller ../../lib/NWN2/include ${VCPKG_INCLUDE_DIR} ${VCPKG_LIB_DIR_DEBUG}/mswud ${VCPKG_LIB_DIR_RELEASE}/mswu)
target_link_directories(hhGui PRIVATE
        ../lib/NWN2 ${VCPKG_LIB_DIR_DEBUG} ${VCPKG_LIB_DIR_RELEASE})
target_link_libraries(hhGui PRIVATE
        comctl32.lib rpcrt4.lib ws2_32.lib wsock32.lib
        detours.lib
        wxmsw31ud_core.lib wxbase31ud_net.lib wxbase31ud.lib wxregexud.lib jpegd.lib libpng16d.lib zlib.lib)
