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
        add_library(so_${module} OBJECT ${module}.c)
    else()
        add_library(so_${module} MODULE ${module}.c)
        set_target_properties(so_${module}
            PROPERTIES PREFIX ""
        )
    endif()
    set_target_properties(so_${module}
        PROPERTIES OUTPUT_NAME ${module}
    )
endforeach(module)
