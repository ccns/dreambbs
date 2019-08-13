#!/bin/bash
BBSHOME=${BBSHOME:-"/home/bbs"}
declare -i a
declare -i b
declare -i c
cd "${BBSHOME}/run" || exit 1
if (test -e NOUSRDIR.log); then
    rm NOUSRDIR.log
fi
cd "${BBSHOME}/usr" || exit 1
for i in a b c d e f g h i j k l m n o p q r s t u v w x y z; do
    cd "${BBSHOME}/usr/$i" || continue
    for usr in */; do
        cd "${BBSHOME}/usr/$i/$usr" || continue
            if (! (test -d @)); then
                mkdir @
            fi
        if (! (test -e .DIR) || ! (test -e .DIR.o)); then
            echo "$usr" >> "${BBSHOME}/run/NOUSRDIR.log"
            if ! (test -e .DIR); then
                echo "NO .DIR" >> "${BBSHOME}/run/NOUSRDIR.log"
                let a++
            fi
            if ! (test -e .DIR.o); then
                echo "NO .DIR.o" >> "${BBSHOME}/run/NOUSRDIR.log"
                let b++
            fi
        fi
        let c++
    done
done

#cd "${BBSHOME}/usr/$1" || exit 1
#for file in */; do
#    cd "${BBSHOME}/usr/$1/$file" || continue
#    if (! (test -e .DIR) || ! (test -e .DIR.o)); then
#        echo $file >> "${BBSHOME}/run/USRNODIR.log"
#        if ! (test -e .DIR); then
#            echo "NO .DIR" >> "${BBSHOME}/run/USRNODIR.log"
#            let a++
#        fi
#        if ! (test -e .DIR.o); then
#            echo "NO .DIR.o" >> "${BBSHOME}/run/USRNODIR.log"
#            let b++
#        fi
#    fi
#    let c++
#done

{
    echo "$a user mail(s) have no .DIR"
    echo "$b user mail(s) have no .DIR.o"
    echo "$c user(s) are checked"
} >> "${BBSHOME}/run/NOUSRDIR.log"
#mail -s 'DIR.log' *********@gmail.com < "${BBSHOME}/USRNODIR.log"
