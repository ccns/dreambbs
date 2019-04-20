#!/usr/bin/env bash
declare -i a
declare -i b
declare -i c
#declare -i d
cd /home/bbs/run || exit 1
if (test -e NOBRDDIR.log); then
    rm NOBRDDIR.log
fi
cd /home/bbs/brd || exit 1
for file in */; do
    cd "/home/bbs/brd/$file" || continue
    if (! (test -e .DIR) || ! (test -e .DIR.o)); then
        echo "-- $file" >> /home/bbs/run/NOBRDDIR.log
        if ! (test -e .DIR); then
            echo "NO .DIR" >> /home/bbs/run/NOBRDDIR.log
            let a++
        fi
        if ! (test -e .DIR.o); then
            echo "NO .DIR.o" >> /home/bbs/run/NOBRDDIR.log
            let b++
        fi
    fi
#   for folder in 0 1 2 3 4 5 6 7 8 9 A B C D E F G H I J K L M N O P Q R S T U V; do
#       if ! (test -d $folder); then
#           echo "=== $file lose some folder" >> /home/bbs/NODIR.log
#           let d++;
#           break
#       fi
#   done
    let c++
done
{
    echo "$a board(s) have no .DIR"
    echo "$b board(s) have no .DIR.o"
    echo "$c board(s) are checked"
} >> /home/bbs/run/NOBRDDIR.log
#echo "$d board(s) lose folder" >> /home/bbs/NODIR.log
#mail -s 'DIR.log' *********@gmail.com < /home/bbs/NODIR.log
#mail -s 'DIR.log' *********@gmail.com < /home/bbs/NODIR.log
