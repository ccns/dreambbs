#!/bin/sh

set -eux

################### test f_mv() 
touch before.txt
printf "putsometextin" >> before.txt
test "$(uname -o)" = "GNU/Linux" && sha256_before=$(sha256sum before.txt |awk '{print $1}')
test "$(uname -o)" = "FreeBSD"   && sha256_before=$(sha256 before.txt |awk '{print $1}')
./lib_f_mv before.txt after.txt
test "$(uname -o)" = "GNU/Linux" && sha256_after=$(sha256sum after.txt |awk '{print $1}')
test "$(uname -o)" = "FreeBSD"   && sha256_after=$(sha256 after.txt |awk '{print $1}')
test "$sha256_before" = "$sha256_after" && printf "\033[1;32mtest f_mv done ok!\033[0m\n" 
rm after.txt
