# ------------------------------------------------------- #
#  maple/CMakeLists.txt ( NCKU CCNS WindTop-DreamBBS 3.0 )#
# ------------------------------------------------------- #
#  Author: Wei-Cheng Yeh (IID) <iid@ccns.ncku.edu.tw>     #
#  Target: CMakeLists for DreamBBS main programs          #
#  Create: 2019/11/30                                     #
# ------------------------------------------------------- #

include(${PROJECT_SOURCE_DIR}/so/so.cmake)

EXPORT_CONF(${CMAKE_CURRENT_SOURCE_DIR}/make_export.conf)

add_executable(bbsd
    acct.c bbsd.c board.c cache.c edit.c
    gem.c mail.c menu.c more.c post.c
    talk.c visio.c xover.c socket.c popupmenu.c
    window.c myfavorite.c
)
target_compile_options(bbsd PRIVATE -imacros${EXPORT_MAPLE})

if(USE_BBSLUA)
    target_sources(bbsd PRIVATE bbslua.c)
    if(NOT USE_LUAJIT)
        target_sources(bbsd PRIVATE bbsluaext.c)
    endif()
    target_compile_options(bbsd PRIVATE ${LUA_CFLAGS})
    target_link_libraries(bbsd PRIVATE ${LUA_LDFLAGS})
endif()

if(USE_BBSRUBY)
    target_sources(bbsd PRIVATE bbsruby.c)
    target_compile_options(bbsd PRIVATE ${RUBY_CFLAGS})
    target_link_libraries(bbsd PRIVATE ${RUBY_LDFLAGS})
endif()

if(USE_PFTERM)
    target_sources(bbsd PRIVATE pfterm.c)
endif()

if(USE_PMORE)
    target_sources(bbsd PRIVATE pmore.c)
endif()

if(NO_SO)
    foreach(module IN LISTS SO)
        unset(so_${module}_LIB_DEPENDS CACHE)
        target_sources(bbsd PRIVATE $<TARGET_OBJECTS:so_${module}>)
    endforeach()
endif()

add_executable(xchatd xchatd.c)

install(
    TARGETS bbsd xchatd
    DESTINATION bin${BBSVER}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
install(
    CODE "execute_process(COMMAND chown -R ${BBSUSR}:${BBSGROUP} ${CMAKE_INSTALL_PREFIX}/bin${BBSVER})"
    CODE "string(TIMESTAMP BUILDTIME %s)"
    CODE "execute_process(COMMAND mv -f ${CMAKE_INSTALL_PREFIX}/bin${BBSVER}/bbsd ${CMAKE_INSTALL_PREFIX}/bin${BBSVER}/bbsd.\${BUILDTIME})"
    CODE "execute_process(COMMAND ln -sv ${CMAKE_INSTALL_PREFIX}/bin${BBSVER}/bbsd.\${BUILDTIME} ${CMAKE_INSTALL_PREFIX}/bin${BBSVER}/bbsd)"
)
