#!/bin/bash
declare -i a
declare -i b
cd /home/bbs/run
if (test -e NOACCT.log);then
  rm NOACCT.log
fi
cd /home/bbs/usr
for i in a b c d e f g h i j k l m n o p q r s t u v w x y z
do
        cd /home/bbs/usr/$i
        for usr in $(ls)
        do
                cd /home/bbs/usr/$i/$usr
                if !(test -e .ACCT);then
                        echo $usr >> /home/bbs/run/NOACCT.log
                        let a++
                fi
                let b++
        done
done
echo "$a user(s) have no ACCT" >> /home/bbs/run/NOACCT.log
echo "$b user(s) is checked" >> /home/bbs/run/NOACCT.log
#mail -s 'NOACCT.log' *********@gmail.com < /home/bbs/NOACCT.log
#rm /home/bbs/NOACCT.log

