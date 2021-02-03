# ------------------------------------------------------- #
#  scripts/CMakeLists.txt (NCKU CCNS WindTop-DreamBBS 3.0)#
# ------------------------------------------------------- #
#  Author: Wei-Cheng Yeh (IID) <iid@ccns.ncku.edu.tw>     #
#  Target: CMakeLists for scripts                         #
#  Create: 2019/11/30                                     #
# ------------------------------------------------------- #

set(SH
    checkACCT.sh  checkbrdDIR.sh  checkusrDIR.sh  clean.sh  start.sh  top.sh
)
set(WSPROXY
    wsproxy/wsproxy.lua
)

foreach(target IN LISTS SH)
    configure_file(
        ${target}.in ${target}
        @ONLY NEWLINE_STYLE LF
    )
    install(
        PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${target}
        DESTINATION sh${BBSVER}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
    )
endforeach(target)

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
