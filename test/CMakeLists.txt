# ------------------------------------------------------- #
#  test/CMakeLists.txt ( NCKU CCNS WindTop-DreamBBS 3.0 ) #
# ------------------------------------------------------- #
#  Author: 37586669+IepIweidieng@users.noreply.github.com #
#  Target: CMakeLists for test programs                   #
#  Create: 2019/11/30                                     #
# ------------------------------------------------------- #

set(LIBSTR
    lib_str_decode lib_str_pat lib_str_xor lib_from_parse
)

set(LIBFILE
    lib_f_mv lib_f_cp lib_f_rm
)

set(LIBOTHER
    lib_xsort lib_splay testsize
)

enable_testing()

foreach(target IN LISTS LIBSTR LIBOTHER)
    add_executable(${target} ${target}.c)
    add_test(NAME test_${target} COMMAND ${target})
endforeach(target)

foreach(target IN LISTS LIBFILE)
    add_executable(${target} ${target}.c)
endforeach(target)
add_test(NAME scripttest COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/testscript.sh)
