cmake_minimum_required(VERSION 3.21)

PROJECT(dmtypetraits CXX)

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
INCLUDE(cmake/ModuleImport.cmake)
INCLUDE(cmake/ModuleCompileOptions.cmake)
ModuleSetCompileOptions()

SET(DMTYPETRAITS_VERSION_MAJOR "1")
SET(DMTYPETRAITS_VERSION_MINOR "0")
SET(DMTYPETRAITS_VERSION_PATCH "1")
SET(DMTYPETRAITS_VERSION "${DMTYPETRAITS_VERSION_MAJOR}.${DMTYPETRAITS_VERSION_MINOR}.${DMTYPETRAITS_VERSION_PATCH}")

MESSAGE(STATUS "VERSION: ${DMTYPETRAITS_VERSION}")

OPTION(USE_DMTYPETRAITS "use dmtypetraits" OFF)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/include/dmtypetraits_config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/dmtypetraits_config.h)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

if(MSVC)
    add_definitions(/Zc:__cplusplus)
endif()

enable_testing()

ModuleImportAll("thirdparty")

InterfaceImport("libdmtypetraits" "src/libdmtypetraits" "")

if(PROJECT_IS_TOP_LEVEL)
    ExeImportAndTest("test" "libdmtypetraits;dmtest")

    ExeImportAndTest("examples" "libdmtypetraits")
endif()

AddInstall("libdmtypetraits" "${CMAKE_CURRENT_SOURCE_DIR}/include")
