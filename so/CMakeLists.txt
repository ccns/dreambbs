# ------------------------------------------------------ #
#  so/CMakeLists.txt  ( NCKU CCNS WindTop-DreamBBS 3.0 ) #
# ------------------------------------------------------ #
#  Author: 37586669+IepIweidieng@users.noreply.github.com#
#  Target: CMakeLists for DreamBBS shared libraries      #
#  Create: 2019/11/30                                    #
# ------------------------------------------------------ #

include(${PROJECT_SOURCE_DIR}/so/so.cmake)

set_property(DIRECTORY PROPERTY LINK_LIBRARIES "")

foreach(module IN LISTS SO)
    if(NO_SO)
        unset(so_${module}_LIB_DEPENDS CACHE)
        add_library(so_${module} OBJECT ${module}.c)
    else()
        add_library(so_${module} MODULE ${module}.c)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            target_link_libraries(so_${module} PRIVATE dao -m32)
        else()
            target_link_libraries(so_${module} PRIVATE dao)
        endif()
        target_compile_definitions(so_${module} PRIVATE
            DL_CURRENT_MODULE_STR="$<TARGET_FILE_NAME:so_${module}>"
        )
        set_target_properties(so_${module}
            PROPERTIES PREFIX ""
        )
        install(
            TARGETS so_${module}
            DESTINATION bin${BBSVER}
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
        )
    endif()

    target_compile_options(so_${module} PRIVATE -imacros${EXPORT_MAPLE})

    if(USE_BBSLUA)
        target_compile_options(so_${module} PRIVATE ${LUA_CFLAGS})
    endif()

    if(USE_BBSRUBY)
        target_compile_options(so_${module} PRIVATE ${RUBY_CFLAGS})
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
