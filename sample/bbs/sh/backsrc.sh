#!/bin/sh
# �ƥ���Ƨ�
mkdir -p /var/backup/system

# �w���ƥ� source �� script
tar zcvf /var/backup/system/src`date +%m%d`.tgz src/ &
tar zcvf /var/backup/system/html`date +%m%d`.tgz html/ &
