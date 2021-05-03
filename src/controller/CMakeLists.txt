project(hhController VERSION 1.0)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DWIN32 /D__WXMSWUD__ /D_UNICODE /D_WINDOWS /GR /EHsc")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /DWIN32 /D__WXMSWU__ /D_UNICODE /D_WINDOWS /GR /EHsc")
set(CMAKE_EXE_LINKER_FLAGS "/NODEFAULTLIB:MSVCRT")
set(SOURCE_FILES main.cpp controller.cpp ../misc/log.cpp service.cpp udp.cpp)
set(VCPKG_INCLUDE_DIR C:/opt/vcpkg/installed/x86-windows-static-md/include/)
set(VCPKG_LIB_DIR_DEBUG C:/opt/vcpkg/installed/x86-windows-static-md/debug/lib/)
set(VCPKG_LIB_DIR_RELEASE C:/opt/vcpkg/installed/x86-windows-static-md/lib/)

add_executable(hhController ${SOURCE_FILES})

target_include_directories(hhController PRIVATE
        ../controller ../../lib/NWN2/include ${VCPKG_INCLUDE_DIR} ${VCPKG_LIB_DIR_DEBUG}/mswud ${VCPKG_LIB_DIR_RELEASE}/mswu)
target_link_directories(hhController PRIVATE
        ../lib/NWN2 ${VCPKG_LIB_DIR_DEBUG} ${VCPKG_LIB_DIR_RELEASE})
target_link_libraries(hhController PRIVATE
        ws2_32.lib wsock32.lib
        detours.lib
        wxmsw31ud_core.lib wxbase31ud.lib wxregexud.lib)
