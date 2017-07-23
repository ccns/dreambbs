#!/bin/sh
LOGFILE="/home/bbs/run/backup.log"
echo -n "Backup Start     : " > $LOGFILE
date >> $LOGFILE
cd /home/bbs/
MON=`/bin/date -v-2m "+%m"`
tar zcvf /var/backup/system/bin`date +%m%d`.tgz bin/ &
tar zcvf /var/backup/system/USR`date +%m%d`.tgz .USR &
tar zcvf /var/backup/system/BRD`date +%m%d`.tgz .BRD &
tar zcvf /var/backup/system/etc`date +%m%d`.tgz etc/ &
tar zcvf /var/backup/system/innd`date +%m%d`.tgz innd/ &
tar zcvf /var/backup/system/newboard`date +%m%d`.tgz newboard/ &
rm -rf /var/backup/brd1 
rm -rf /var/backup/gem1 
rm -rf /var/backup/usr1
mv /var/backup/brd/ /var/backup/brd1/
mv /var/backup/gem/ /var/backup/gem1/
mv /var/backup/usr/ /var/backup/usr1/
mkdir -p /var/backup/brd 
mkdir -p /var/backup/gem 
mkdir -p /var/backup/usr
echo -n "Backup Usr Start : " >> $LOGFILE
date >> $LOGFILE 
bin/backupusr
echo -n "Backup Usr End   : " >> $LOGFILE
date >> $LOGFILE 
echo -n "Backup Brd Start : " >> $LOGFILE
date >> $LOGFILE 
bin/backupbrd
echo -n "Backup Brd End   : " >> $LOGFILE
date >> $LOGFILE
echo -n "Backup Gem Start : " >> $LOGFILE
date >> $LOGFILE 
bin/backupgem
echo -n "Backup Gem End   : " >> $LOGFILE
date >> $LOGFILE
rm -f /var/backup/system/bin$MON* &
rm -f /var/backup/system/BRD$MON* &
rm -f /var/backup/system/etc$MON* &
rm -f /var/backup/system/innd$MON* &
rm -f /var/backup/system/USR$MON* &
rm -f /var/backup/system/gem$MON* &
rm -f /var/backup/system/newboard$MON* &

# 定期備份 gem 的 script
mkdir -p backupgem
cp gem/.* backupgem/
list="0 1 2 3 4 5 6 7 8 9 @ A B C D E F G H I J K L M N O P Q R S T U V "
for i in $list
do
mkdir -p backupgem/$i
cp -rf gem/$i/* backupgem/$i/
done
tar zcvf /var/backup/system/gem`date +%m%d`.tgz backupgem/ 
rm -rf backupgem/
echo -n "Backup End       : " >> $LOGFILE
date >> $LOGFILE
mail bbs@bbs.yzu.edu.tw < /home/bbs/run/backup.log
