# ------------------------------------------------------ #
#  lib/CMakeLists.txt ( NCKU CCNS WindTop-DreamBBS 3.0 ) #
# ------------------------------------------------------ #
#  Author: 37586669+IepIweidieng@users.noreply.github.com#
#  Target: CMakeLists for DreamBBS library               #
#  Create: 2019/11/30                                    #
# ------------------------------------------------------ #

set_property(DIRECTORY PROPERTY LINK_LIBRARIES "")

add_library(dao STATIC
    acl.c       file.c    isnot.c    radix32.c   shm.c
    dl_lib.c    record.c  splay.c    date.c      xsort.c
    attr_lib.c  dns.c       header.c  rfc2047.c  string.c    xwrite.c
    proc.c
)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_link_libraries(dao PRIVATE -m32)
endif()
