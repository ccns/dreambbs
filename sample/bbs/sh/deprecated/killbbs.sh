#!/bin/sh
# �M�����W�ϥΪ̻Pshared memory
kill -9 `ps -auxwww | grep bbsd | awk '{print $2}'`
ipcrm -M 2999
ipcrm -M 2997
ipcrm -M 1998
ipcrm -M 6000
ipcrm -M 3999
ipcrm -M 5000
ipcrm -M 4000
