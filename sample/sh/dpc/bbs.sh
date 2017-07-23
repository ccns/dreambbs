#!/bin/sh
case "$1" in
start)
        [ -x /home/bbs/bin/account ] && {
                su -l bbs -c 'exec /home/bbs/bin/account'
                su -l bbs -c 'exec /home/bbs/bin/camera'
		su -l bbs -c 'exec /home/bbs/bin/acpro'
		su -l bbs -c 'exec /home/bbs/bin/makefw'
                su -l bbs -c 'exec /home/bbs/bin/xchatd'
                su -l bbs -c 'exec /home/bbs/innd/innbbsd'
                /home/bbs/bin/bbsd
                /home/bbs/bin/bguard
                /home/bbs/bin/bmtad
                /home/bbs/bin/bpop3d
                /home/bbs/bin/gemd
        }
        ;;
stop)
        [ -x /home/bbs/bin/account ] && {
                killall -9 xchatd
                su -l bbs -c 'exec /home/bbs/innd/ctlinnbbsd shutdown'
                killall -9 bbsd
                killall -9 bguard
                killall -9 gemd
                killall -9 bpop3d
for i in `ipcs | grep m | grep bbs | awk '{print $3}'` 
do
  if [ $OSTYPE = "FreeBSD" ]; then
         ipcrm -M $i
  fi
done
	 ipcrm -S 2000
        }
        ;;
reload)
	su -l bbs -c 'exec /home/bbs/innd/ctlinnbbsd reload'
	kill `tail -n -1 /home/bbs/run/bbs.pid | awk '{print $1}'`
	;;
killinnbbsd)
	kill -9 `ps -auxwww | grep innbbsd | awk '{print $2}'`
	kill -9 `ps -auxwww | grep bbslink | awk '{print $2}'`
	kill -9 `ps -auxwww | grep bbsnnrp | awk '{print $2}'`
	;;
*)
        echo "Usage: bbs.sh {start|stop|reload|killinnbbsd}" >&2
        exit 64
        ;;
esac
