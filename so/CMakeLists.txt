# ------------------------------------------------------ #
#  so/CMakeLists.txt  ( NCKU CCNS WindTop-DreamBBS 3.0 ) #
# ------------------------------------------------------ #
#  Author: 37586669+IepIweidieng@users.noreply.github.com#
#  Target: CMakeLists for DreamBBS shared libraries      #
#  Create: 2019/11/30                                    #
# ------------------------------------------------------ #

include(${PROJECT_SOURCE_DIR}/so/so.cmake)

add_compile_options(-imacros${EXPORT_MAPLE})

if(USE_BBSLUA)
    add_compile_options(${LUA_CFLAGS})
endif()

if(USE_BBSRUBY)
    add_compile_options(${RUBY_CFLAGS})
endif()

set_property(DIRECTORY PROPERTY LINK_LIBRARIES "")
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    link_libraries(dao -L/usr/lib32 -L/lib32 c -m32)
else()
    link_libraries(dao c -m32)
endif()

foreach(module IN LISTS SO)
    if(NO_SO)
        unset(so_${module}_LIB_DEPENDS CACHE)
        add_library(so_${module} OBJECT ${module}.c)
    else()
        add_library(so_${module} MODULE ${module}.c)
        set_target_properties(so_${module}
            PROPERTIES PREFIX ""
        )
        install(
            TARGETS so_${module}
            DESTINATION bin${BBSVER}
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
        )
    endif()
    set_target_properties(so_${module}
        PROPERTIES OUTPUT_NAME ${module}
    )
endforeach(module)

if(NOT NO_SO)
    install(
        CODE "execute_process(COMMAND chown -R ${BBSUSR}:${BBSGROUP} ${CMAKE_INSTALL_PREFIX}/bin${BBSVER})"
    )
endif()
