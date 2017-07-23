#!/bin/sh
# 清除站上使用者與shared memory
kill -9 `ps -auxwww | grep bbsd | awk '{print $2}'`
ipcrm -M 2999
ipcrm -M 2997
ipcrm -M 1998
ipcrm -M 6000
ipcrm -M 3999
ipcrm -M 5000
ipcrm -M 4000
