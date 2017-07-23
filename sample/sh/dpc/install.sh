#!/bin/sh
# 1.請先修改 src/include/config.h 中的
#     BOARDNAME、NICKNAME、MYHOSTNAME、SYSOPNICK
# 2.修改 sed.sh 並執行 /bin/sh sed.sh
# 3.執行 /bin/sh install.sh
#
#                                      by statue.bbs@bbs.yzu.edu.tw

echo "build bbs user and group, remember to passwd bbs";
mkdir -p /home/bbs

echo "Step 1:";
echo "#######build bbs account";
if [ $OSTYPE = "FreeBSD" ]; then
  if [ `grep -c "bbs:" /etc/group` != 1 ]; then
    echo "bbs:*:99:" >> /etc/group
  fi
  if [ `grep -c "9999:99" /etc/passwd` != 1 ]; then
    echo -n "bbs::9999:99::0:0:bbs:/home/bbs:" >> /etc/master.passwd
    echo "/usr/local/bin/tcsh" >> /etc/master.passwd
    pwd_mkdb -p -d /etc /etc/master.passwd
  fi
else if [ $OSTYPE = "linux-gnu" -o $OSTYPE = "Linux" ]; then
  if [ `grep -c "bbs:" /etc/group` != 1 ]; then
    echo "bbs:x:999:" >> /etc/group
  fi
  useradd -g bbs -u 9999 bbs
     fi
fi
echo $ostype

echo "Step 3:";
echo "#######build bbs";
cd /home/bbs
sh src/install.sh

echo "Step 4:";
if [ `grep -c "WindTopBBS" /etc/inetd.conf` = 1  ]; then
echo "#######/etc/inetd.conf had configed"
else
echo "#######cancel telnet and finger service";
cat /etc/inetd.conf | sed 's/^telnet/#telnet/g' > /etc/inetd.conf.sed
cp /etc/inetd.conf.sed /etc/inetd.conf
cat /etc/inetd.conf | sed 's/^finger/#finger/g' > /etc/inetd.conf.sed
cp /etc/inetd.conf.sed /etc/inetd.conf
rm /etc/inetd.conf.sed
echo "#######add WindTopBBS /etc/inetd.conf";
echo "# WindTopBBS inetd.conf config" >> /etc/inetd.conf
echo -n "telnet	stream	tcp	wait	bbs	" >> /etc/inetd.conf
echo "/home/bbs/bin/bbsd	bbsd -i" >> /etc/inetd.conf
echo -n "finger	stream	tcp	wait	bbs	" >> /etc/inetd.conf
echo "/home/bbs/bin/bguard	bguard -i" >> /etc/inetd.conf
echo -n "pop3	stream	tcp	wait	bbs	" >> /etc/inetd.conf
echo "/home/bbs/bin/bpop3d	bpop3d -i" >> /etc/inetd.conf
echo -n "gopher	stream	tcp	wait	bbs	" >> /etc/inetd.conf
echo "/home/bbs/bin/gemd	gemd -i" >> /etc/inetd.conf
echo -n "smtp	stream	tcp	wait	bbs	" >> /etc/inetd.conf
echo "/home/bbs/bin/bmtad	bmtad -i" >> /etc/inetd.conf
echo -n "xchat	stream	tcp	wait	bbs	" >> /etc/inetd.conf
echo "/home/bbs/bin/xchatd	xchatd -i" >> /etc/inetd.conf
echo -n "bbsnntp	stream	tcp	wait	bbs	" >> /etc/inetd.conf
echo "/home/bbs/innd/innbbsd	innbbsd -i" >> /etc/inetd.conf

echo -n "telnet2	stream	tcp	nowait	root	" >> /etc/inetd.conf
if [ $OSTYPE = "FreeBSD" ]; then
echo "/usr/libexec/telnetd	telnetd" >> /etc/inetd.conf
else if [ $OSTYPE = "linux-gnu" -o $OSTYPE = "Linux" ]; then
echo "/usr/sbin/tcpd	in.telnetd" >> /etc/inetd.conf
     fi
fi

fi

if [ `grep -c "WindTopBBS" /etc/services` = 1  ]; then
echo "#######/etc/services had configed"
else
echo "#######add WindTopBBS /etc/services";
echo "# WindTopBBS services config" >> /etc/services
echo "telnet2		6666/tcp" >> /etc/services
echo "telnet2		6666/udp" >> /etc/services
echo "xchat		3838/tcp" >> /etc/services
echo "xchat		3838/udp" >> /etc/services
echo "bbsnntp		7777/tcp" >> /etc/services
echo "bbsnntp		7777/udp" >> /etc/services
fi

echo "#######change user and group owner to bbs.bbs"
chown -R bbs.bbs /home/bbs

echo "Step 5:";
echo "#######kill not use bbsd process";
kill -9 `ps -auxwww | grep bbsd | awk '{print $2}'`

echo "#######ipcrm -M all shared memory"
for i in `ipcs | grep bbs | awk '{print $2}'`
do
  if [ $OSTYPE = "FreeBSD" ]; then
         ipcrm -M $i
  else if [ $OSTYPE = "linux-gnu" -o $OSTYPE = "Linux" ]; then
         ipcrm shm $i
       fi
  fi
done

echo "#######run shared memory";
su bbs -c '/home/bbs/bin/camera'
su bbs -c 'touch /home/bbs/log/mail-00`date +%m%d`'
su bbs -c 'touch /home/bbs/log/innbbs-00`date +%m%d`'
su bbs -c 'touch /home/bbs/log/chicken-00`date +%m%d`'
su bbs -c 'touch /home/bbs/log/ordersongs-00`date +%m%d`' 
su bbs -c '/home/bbs/bin/account'
su bbs -c '/home/bbs/bin/acpro'

echo "#######rerun inetd";
kill -1 `ps -auxwww | grep inetd | awk '{print $2}'`
