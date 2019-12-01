# ------------------------------------------------------- #
#  scripts/CMakeLists.txt (NCKU CCNS WindTop-DreamBBS 3.0)#
# ------------------------------------------------------- #
#  Author: 37586669+IepIweidieng@users.noreply.github.com #
#  Target: CMakeLists for scripts                         #
#  Create: 2019/11/30                                     #
# ------------------------------------------------------- #

set(SH
    checkACCT.sh  checkbrdDIR.sh  checkusrDIR.sh  clean.sh  start.sh  top.sh
)
set(WSPROXY
    wsproxy/wsproxy.lua
)

install(
    PROGRAMS ${SH}
    DESTINATION sh${BBSVER}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
)
install(
    CODE "execute_process(COMMAND chown -R ${BBSUSR}:${BBSGROUP} ${CMAKE_INSTALL_PREFIX}/sh${BBSVER})"
)

install(
    PROGRAMS ${WSPROXY}
    DESTINATION wsproxy${BBSVER}
)
install(
    CODE "execute_process(COMMAND chown -R ${BBSUSR}:${BBSGROUP} ${CMAKE_INSTALL_PREFIX}/wsproxy${BBSVER})"
)
