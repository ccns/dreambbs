# ------------------------------------------------------- #
#  sample/CMakeLists.txt ( NCKU CCNS DreamBBS 3.0 )       #
# ------------------------------------------------------- #
#  Author: Wei-Cheng Yeh (IID) <iid@ccns.ncku.edu.tw>     #
#  Target: CMakeLists for DreamBBS sample files           #
#  Create: 2020-10-15                                     #
# ------------------------------------------------------- #

set(DEST
    bbsd.service bbsd_unixsocket.service
    startbbs.service xchatd.service
    bbs-sshd.service
    crontab
)

foreach(target IN LISTS DEST)
    configure_file(
        ${target}.in ${target}
        @ONLY NEWLINE_STYLE LF
    )
endforeach(target)
