# ------------------------------------------------------ #
#  lib/CMakeLists.txt ( NCKU CCNS WindTop-DreamBBS 3.0 ) #
# ------------------------------------------------------ #
#  Author: Wei-Cheng Yeh (IID) <iid@ccns.ncku.edu.tw>    #
#  Target: CMakeLists for DreamBBS library               #
#  Create: 2019/11/30                                    #
# ------------------------------------------------------ #

set_property(DIRECTORY PROPERTY LINK_LIBRARIES "")

add_library(dao STATIC
    acl.c       file.c    isnot.c    radix32.c   shm.c
    dl_lib.c    record.c  splay.c    date.c      xsort.c
    attr_lib.c  dns.c       header.c  string.c    xwrite.c
    proc.c      passwd.c  mime.c     logger.c
)
