#!/bin/sh

set -eux

################### test f_mv() 
touch before_mv.txt
printf "putsometextin" >> before_mv.txt
test "$(uname -o)" = "GNU/Linux" && sha256_before_mv=$(sha256sum before_mv.txt |awk '{print $1}')
test "$(uname -o)" = "FreeBSD"   && sha256_before_mv=$(sha256 before_mv.txt |awk '{print $1}')
./lib_f_mv before_mv.txt after_mv.txt
test "$(uname -o)" = "GNU/Linux" && sha256_after_mv=$(sha256sum after_mv.txt |awk '{print $1}')
test "$(uname -o)" = "FreeBSD"   && sha256_after_mv=$(sha256 after_mv.txt |awk '{print $1}')
test "$sha256_before_mv" = "$sha256_after_mv" && (>&2 printf "\033[1;32mtest f_mv ok!\033[0m\n")
rm after_mv.txt

################### test f_cp()
touch before_cp.txt
printf "putanoterrrrrtextin" >> before_cp.txt
test "$(uname -o)" = "GNU/Linux" && sha256_before_cp=$(sha256sum before_cp.txt |awk '{print $1}')
test "$(uname -o)" = "FreeBSD"   && sha256_before_cp=$(sha256 before_cp.txt |awk '{print $1}')
./lib_f_cp before_cp.txt after_cp.txt
test "$(uname -o)" = "GNU/Linux" && sha256_after_cp=$(sha256sum after_cp.txt |awk '{print $1}')
test "$(uname -o)" = "FreeBSD"   && sha256_after_cp=$(sha256 after_cp.txt |awk '{print $1}')
test "$sha256_before_cp" = "$sha256_after_cp" && (>&2 printf "\033[1;32mtest f_cp ok!\033[0m\n")
rm before_cp.txt after_cp.txt
