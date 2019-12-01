# ------------------------------------------------------- #
#  maple/CMakeLists.txt ( NCKU CCNS WindTop-DreamBBS 3.0 )#
# ------------------------------------------------------- #
#  Author: 37586669+IepIweidieng@users.noreply.github.com #
#  Target: CMakeLists for DreamBBS main programs          #
#  Create: 2019/11/30                                     #
# ------------------------------------------------------- #

include(${PROJECT_SOURCE_DIR}/so/so.cmake)

add_compile_options(-imacros${EXPORT_MAPLE})

if(USE_BBSLUA)
    list(APPEND SRC_EXTRA bbslua.c)
    if(NOT USE_LUAJIT)
        list(APPEND SRC_EXTRA bbsluaext.c)
    endif()
    add_compile_options(${LUA_CFLAGS})
    link_libraries(${LUA_LDFLAGS})
endif()

if(USE_BBSRUBY)
    list(APPEND SRC_EXTRA bbsruby.c)
    add_compile_options(${RUBY_CFLAGS})
    link_libraries(${RUBY_LDFLAGS})
endif()

if(USE_PFTERM)
    list(APPEND SRC_EXTRA pfterm.c)
endif()

if(USE_PMORE)
    list(APPEND SRC_EXTRA pmore.c)
endif()

if(NO_SO)
    foreach(module IN LISTS SO)
        unset(so_${module}_LIB_DEPENDS CACHE)
        list(APPEND SRC_EXTRA $<TARGET_OBJECTS:so_${module}>)
    endforeach()
endif()

add_executable(xchatd xchatd.c)

add_executable(bbsd
    acct.c bbsd.c board.c cache.c edit.c
    gem.c mail.c menu.c more.c post.c banmail.c
    talk.c visio.c xover.c socket.c popupmenu.c
    window.c myfavorite.c
    ${SRC_EXTRA}
)
if(NOT NO_SO)
    foreach(module IN LISTS SO)
        target_link_libraries(bbsd PRIVATE so_${module})
    endforeach()
endif()
