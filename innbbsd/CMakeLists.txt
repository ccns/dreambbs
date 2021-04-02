# ------------------------------------------------------- #
#  innbbsd/CMakeLists.txt (NCKU CCNS WindTop-DreamBBS 3.0)#
# ------------------------------------------------------- #
#  Author: Wei-Cheng Yeh (IID) <iid@ccns.ncku.edu.tw>     #
#  Target: CMakeLists for Newsgroup program innbbsd     #
#  Create: 2019/11/30                                     #
# ------------------------------------------------------- #

add_executable(bbslink
    bbslink.c bbslib.c convcode.c inntobbs.c rec_article.c nocem.c history.c
)
add_executable(innbbsd
    channel.c bbslib.c convcode.c inntobbs.c rec_article.c nocem.c history.c
)

install(
    TARGETS bbslink innbbsd
    DESTINATION innd${BBSVER}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
)
install(
    CODE "execute_process(COMMAND chown -R ${BBSUSR}:${BBSGROUP} ${CMAKE_INSTALL_PREFIX}/innd${BBSVER})"
)
