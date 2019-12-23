# ------------------------------------------------------ #
#  util/CMakeLists.txt ( NCKU CCNS WindTop-DreamBBS 3.0 )#
# ------------------------------------------------------ #
#  Author: 37586669+IepIweidieng@users.noreply.github.com#
#  Target: CMakeLists for maintenance tools              #
#  Create: 2019/11/30                                    #
# ------------------------------------------------------ #

set(EXE
    account acl-sort camera expire gem-check gem-index makeUSR
    hdr-dump poststat reaper rmbadmail restorebrd
    mailpost bbsmail topusr acpro addsong userno template restoregem
    checkemail match mailexpire utmp-dump
    stopperm mailtoall clean_acl makefw
    resetvmail counter restoreusr makeusrno outgo redir
    classtable_alert bmw-dump tranBRD
    brdstat brdcheck dump-brdstat base64encode
    msgall transacct showACCT showBRD
    readBENZ readFRIENZ readUSR
    backup brdmail addpost
)

foreach(target IN LISTS EXE)
    add_executable(${target} ${target}.c)
    install(
        TARGETS ${target}
        DESTINATION bin${BBSVER}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
    )
endforeach(target)

install(
    CODE "execute_process(COMMAND chown -R ${BBSUSR}:${BBSGROUP} ${CMAKE_INSTALL_PREFIX}/bin${BBSVER})"
)
