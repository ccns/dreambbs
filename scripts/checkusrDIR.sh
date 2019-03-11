#!/bin/bash
declare -i a
declare -i b
declare -i c
cd /home/bbs/run || exit 1
if (test -e NOUSRDIR.log); then
    rm NOUSRDIR.log
fi
cd /home/bbs/usr || exit 1
for i in a b c d e f g h i j k l m n o p q r s t u v w x y z; do
    cd "/home/bbs/usr/$i" || continue
    for usr in */; do
        cd "/home/bbs/usr/$i/$usr" || continue
            if(! (test -d @)); then
                mkdir @
            fi
        if (! (test -e .DIR) || ! (test -e .DIR.o)); then
            echo "$usr" >> /home/bbs/run/NOUSRDIR.log
            if ! (test -e .DIR); then
                echo "NO .DIR" >> /home/bbs/run/NOUSRDIR.log
                let a++
            fi
            if ! (test -e .DIR.o); then
                echo "NO .DIR.o" >> /home/bbs/run/NOUSRDIR.log
                let b++
            fi
        fi
        let c++
    done
done

#cd "/home/bbs/usr/$1" || exit 1
#for file in */; do
#    cd "/home/bbs/usr/$1/$file" || continue
#    if (! (test -e .DIR) || ! (test -e .DIR.o)); then
#        echo $file >> /home/bbs/run/USRNODIR.log
#        if ! (test -e .DIR); then
#            echo "NO .DIR" >> /home/bbs/run/USRNODIR.log
#            let a++
#        fi
#        if ! (test -e .DIR.o); then
#            echo "NO .DIR.o" >> /home/bbs/run/USRNODIR.log
#            let b++
#        fi
#    fi
#    let c++
#done

{
    echo "$a user mail(s) have no .DIR"
    echo "$b user mail(s) have no .DIR.o"
    echo "$c user(s) are checked"
} >> /home/bbs/run/NOUSRDIR.log
#mail -s 'DIR.log' *********@gmail.com < /home/bbs/USRNODIR.log
