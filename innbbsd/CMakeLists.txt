# ------------------------------------------------------- #
#  innbbsd/CMakeLists.txt (NCKU CCNS WindTop-DreamBBS 3.0)#
# ------------------------------------------------------- #
#  Author: 37586669+IepIweidieng@users.noreply.github.com #
#  Target: CMakeLists for Newsgroup program innbbsd     #
#  Create: 2019/11/30                                     #
# ------------------------------------------------------- #

add_executable(bbslink
    bbslink.c bbslib.c convcode.c inntobbs.c rec_article.c nocem.c history.c
)
add_executable(innbbsd
    channel.c bbslib.c convcode.c inntobbs.c rec_article.c nocem.c history.c
)
