# ------------------------------------------------------- #
#  CMakeLists.txt   ( NCKU CCNS WindTop-DreamBBS 3.0 )    #
# ------------------------------------------------------- #
#  Author: Wei-Cheng Yeh (IID) <iid@ccns.ncku.edu.tw>     #
#  Target: CMakeLists for ALL                             #
#  Create: 2019/11/30                                     #
# ------------------------------------------------------- #

cmake_minimum_required(VERSION 3.1)

# set the project name
project(DreamBBS LANGUAGES C CXX VERSION 3.0)

# For debugging `CMakeLists.txt`
# set(CMAKE_VERBOSE_MAKEFILE ON)

# Mark `CMakeCache.txt` as up-to-date if necessary
set(MARK_CACHE_UPDATED OFF)

# Required cmake modules
include(CheckCCompilerFlag)
include(CheckSymbolExists)
include(FindPkgConfig)

## Toolchain settings

if(NOT USE_CXX)
    # specify the C standard
    add_compile_options(-x c --std=gnu99)
    set(USE_CXX OFF CACHE BOOL "Compile C code using C++ mode")
else()
    # specify the C++ standard
    add_compile_options(-x c++ --std=gnu++14)
    link_libraries(stdc++)
    set(USE_CXX ON CACHE BOOL "Compile C code using C++ mode")
endif()

add_compile_options(-ggdb3 -O0 -pipe)
add_compile_options(-Wall -Wpointer-arith -Wcast-qual -Wwrite-strings -Werror=format -Werror=incompatible-pointer-types -Werror=int-conversion -Werror=pointer-to-int-cast)
include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include)

link_libraries(-L${PROJECT_BINARY_DIR}/lib dao crypt)

## Tool functions

function(GETVALUE conf default hdr output)
    set(valueif "#ifdef ${conf}\n${conf}\n#else\n${default}\n#endif")
    execute_process(
        COMMAND echo ${valueif}
        COMMAND ${CMAKE_C_COMPILER} -x c -E -P -imacros "${hdr}" -
        COMMAND xargs
        OUTPUT_VARIABLE "${output}"
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    set("${output}" "${${output}}" PARENT_SCOPE)
endfunction(GETVALUE)

function(LOADVALUE conf default hdr output)
    if(DEFINED ENV{${conf}})
        set("${output}" "$ENV{${conf}}")
    elseif(DEFINED "${conf}")
        set("${output}" "${${conf}}")
    else()
        GETVALUE("${conf}" "${default}" "${hdr}" "${output}")
    endif()
    set("${output}" "${${output}}" PARENT_SCOPE)
endfunction(LOADVALUE)

set(ID_DEFAULT 9999)

function(GETUID name output)
    execute_process(
        COMMAND sh -c "id -u ${name} || echo ${ID_DEFAULT}"
        OUTPUT_VARIABLE "${output}"
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    set("${output}" "${${output}}" PARENT_SCOPE)
endfunction(GETUID)

function(GETGID name output)
    execute_process(
        COMMAND getent group ${name}
        COMMAND cut -d: -f3 -
        OUTPUT_VARIABLE "${output}"
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if ("${${output}}" STREQUAL "")
        set("${output}" ${ID_DEFAULT})
    endif()
    set("${output}" "${${output}}" PARENT_SCOPE)
endfunction(GETGID)

function(DEF_TEST conf hdr output)
    if("${hdr}" IS_NEWER_THAN ${CMAKE_CACHEFILE_DIR}/CMakeCache.txt)
        set(MARK_CACHE_UPDATED ON)
        unset("${output}" CACHE)
        check_symbol_exists("${conf}" "${hdr}" "${output}")
    endif()
    set("${output}" "${${output}}" PARENT_SCOPE)
endfunction(DEF_TEST)

## Read variables from the configuration C files

set(BBSCONF "${PROJECT_SOURCE_DIR}/dreambbs.conf")
set(BBSCONF_ORIGIN "${PROJECT_SOURCE_DIR}/include/config.h")
set(EXPORT_PROJECT "${PROJECT_SOURCE_DIR}/make_export.conf")
set(EXPORT_MAPLE "${PROJECT_SOURCE_DIR}/maple/make_export.conf")
set(EXPORT_VERINFO "${PROJECT_SOURCE_DIR}/verinfo_export.conf")

if(NOT EXISTS ${EXPORT_PROJECT})
    execute_process(COMMAND touch ${EXPORT_PROJECT})
endif()
if(NOT EXISTS ${EXPORT_MAPLE})
    execute_process(COMMAND touch ${EXPORT_MAPLE})
endif()

# User names and group names
execute_process(COMMAND id -un OUTPUT_VARIABLE BBSUSR_DEFAULT OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND id -gn OUTPUT_VARIABLE BBSGROUP_DEFAULT OUTPUT_STRIP_TRAILING_WHITESPACE)
LOADVALUE(BBSUSR "${BBSUSR_DEFAULT}" ${BBSCONF_ORIGIN} BBSUSR)
LOADVALUE(BBSGROUP "${BBSGROUP_DEFAULT}" ${BBSCONF_ORIGIN} BBSGROUP)
LOADVALUE(WWWGROUP "www-data" ${BBSCONF_ORIGIN} WWWGROUP)

# UIDs and GIDs
GETUID("${BBSUSR}" BBSUID_DEFAULT)
GETGID("${BBSGROUP}" BBSGID_DEFAULT)
GETGID("${WWWGROUP}" WWWGID_DEFAULT)
LOADVALUE(BBSUID "${BBSUID_DEFAULT}" ${BBSCONF_ORIGIN} BBSUID)
LOADVALUE(BBSGID "${BBSGID_DEFAULT}" ${BBSCONF_ORIGIN} BBSGID)
LOADVALUE(WWWGID "${WWWGID_DEFAULT}" ${BBSCONF_ORIGIN} WWWGID)

# BBS path prefixes and suffixes
LOADVALUE(BBSVER_SUFFIX "" ${BBSCONF_ORIGIN} BBSVER)
execute_process(
    COMMAND getent passwd ${BBSUSR}
    COMMAND cut -d: -f6 -
    OUTPUT_VARIABLE BBSUSR_HOME
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
LOADVALUE(BBSHOME "${BBSUSR_HOME}" ${BBSCONF_ORIGIN} BBSHOME)

## Numeric local timezone
execute_process(
    COMMAND date +%z
    OUTPUT_VARIABLE BBSUTCZONE_DEFAULT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
LOADVALUE(BBSUTCZONE "${BBSUTCZONE_DEFAULT}" ${BBSCONF_ORIGIN} BBSUTCZONE)

## `mruby-config` executable path for building BBS-Ruby with mruby
execute_process(
    COMMAND which mruby-config
    OUTPUT_VARIABLE MRB_CONFIG_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
LOADVALUE(MRB_CONFIG "${MRB_CONFIG_PATH}" ${BBSCONF_ORIGIN} MRB_CONFIG)

# Flags for enabling modules
DEF_TEST(M3_USE_PMORE ${BBSCONF} USE_PMORE)
DEF_TEST(M3_USE_PFTERM ${BBSCONF} USE_PFTERM)
DEF_TEST(M3_USE_BBSLUA ${BBSCONF} USE_BBSLUA)
DEF_TEST(M3_USE_BBSRUBY ${BBSCONF} USE_BBSRUBY)
DEF_TEST(BBSLUA_USE_LUAJIT ${BBSCONF} USE_LUAJIT)
DEF_TEST(BBSRUBY_USE_MRUBY ${BBSCONF} USE_MRUBY)

# Flags for disabling shared objects
if(DEFINED ENV{NO_SO})
    set(NO_SO_CLI "$ENV{NO_SO}")
elseif(DEFINED NO_SO)
    set(NO_SO_CLI "${NO_SO}")
endif()

if(CYGWIN)
    set(NO_SO ON)
elseif(DEFINED NO_SO_CLI)
    set(NO_SO "${NO_SO_CLI}")
else()
    LOADVALUE(NO_SO OFF ${EXPORT_MAPLE} NO_SO)
endif()

## Other compile options and link options settings

# Prevent `-Wformat-overflow=` warnings from halting the compilation
set(W_NO_ERROR_FORMAT_OVERFLOW "-Wno-format-overflow -Wno-error=format-overflow -Wformat-overflow")
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${W_NO_ERROR_FORMAT_OVERFLOW}")
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${W_NO_ERROR_FORMAT_OVERFLOW}")
endif()

# Prevent `-Wincompatible-pointer-types-discards-qualifiers` warnings from halting the compilation
set(W_NO_INCOMPATIBLE_POINTER_TYPES_DISCARDS_QUALIFIERS "-Wno-incompatible-pointer-types-discards-qualifiers -Wno-error=incompatible-pointer-types-discards-qualifiers -Wincompatible-pointer-types-discards-qualifiers")
if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${W_NO_INCOMPATIBLE_POINTER_TYPES_DISCARDS_QUALIFIERS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-invalid-source-encoding")
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${W_NO_INCOMPATIBLE_POINTER_TYPES_DISCARDS_QUALIFIERS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-source-encoding")
endif()

if(NOT USE_CXX)
    add_compile_options(-Wstrict-prototypes)
endif()

check_c_compiler_flag(-Wunreachable-code-aggressive CC_HAS_W_UNREACHABLE_CODE_AGGRESSIVE)
if(CC_HAS_W_UNREACHABLE_CODE_AGGRESSIVE)
    add_compile_options(-Wunreachable-code-aggressive)
else()
    add_compile_options(-Wunreachable-code)
endif()

check_c_compiler_flag(-fcolor-diagnostics CC_HAS_F_COLOR_DIAGNOSTICS)
if(CC_HAS_F_COLOR_DIAGNOSTICS)
    add_compile_options(-fcolor-diagnostics)
endif()

# Architecture information
execute_process(
    COMMAND ${CMAKE_C_COMPILER} -dumpmachine
    OUTPUT_VARIABLE MULTIARCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REGEX REPLACE "^(.*)-(.*)-(.*)-(.*)$" \\1-\\3-\\4 MULTIARCH ${MULTIARCH})

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_compile_options(-fPIC)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    link_libraries(resolv ${CMAKE_DL_LIBS} -rdynamic)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    link_libraries(-Wl,-export-dynamic)
endif()

# BBS-Lua & BBS-Ruby make rule definitions

if(USE_BBSLUA)
    if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
        link_libraries(-Wl,--no-as-needed)
    endif()
    if(USE_LUAJIT)
        pkg_search_module(LUA REQUIRED luajit)
    else()
        # Prevent LuaJIT from being chosen
        pkg_search_module(LUA REQUIRED lua-5.1 lua5.1 luajit<-1)
    endif()
endif()

if(USE_BBSRUBY)
    if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
        link_libraries(-Wl,--no-as-needed)
    endif()
    if(USE_MRUBY)
        execute_process(
            COMMAND ${MRB_CONFIG} --cflags
            OUTPUT_VARIABLE MRB_CFLAGS
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        execute_process(
            COMMAND ${MRB_CONFIG} --ldflags
            OUTPUT_VARIABLE MRB_CFLAGS
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        set(RUBY_CFLAGS ${MRB_CFLAGS})
        set(RUBY_LDFLAGS ${MRB_LDFLAGS} mruby m)
    else()
        pkg_search_module(RUBY REQUIRED ruby<=2.2 ruby-2.2)
    endif()
endif()

## Generate configure file
function(EXPORT_CONF export_file)
    execute_process(COMMAND printf "\\033[1;36mGenerating '${export_file}'...\\033[0m\n")
    configure_file(
        ${export_file}.in ${export_file}
        NEWLINE_STYLE LF
    )
    file(READ ${export_file} EXPORT_FILE_CONTENT)
    execute_process(COMMAND printf "\\033[1;33m${EXPORT_FILE_CONTENT}\\033[0m")
endfunction()

EXPORT_CONF(${CMAKE_CURRENT_SOURCE_DIR}/make_export.conf)

add_custom_target(verinfo
    ALL
    sh scripts/verinfo.sh CMake ${MULTIARCH}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
add_compile_options(-imacros "${EXPORT_VERINFO}")

set(CMAKE_INSTALL_PREFIX ${BBSHOME})

add_subdirectory(lib)
add_subdirectory(maple)
add_subdirectory(util)
add_subdirectory(innbbsd)
add_subdirectory(so)
add_subdirectory(scripts)
add_subdirectory(sample)

enable_testing()
add_subdirectory(test)

# Mark the cache file as up-to-date
if(MARK_CACHE_UPDATED AND CMAKE_CACHEFILE_DIR)
    execute_process(COMMAND touch ${CMAKE_CACHEFILE_DIR}/CMakeCache.txt)
endif()
