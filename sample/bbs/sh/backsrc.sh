#!/bin/sh
# 備份資料夾
mkdir -p /var/backup/system

# 定期備份 source 的 script
tar zcvf /var/backup/system/src`date +%m%d`.tgz src/ &
tar zcvf /var/backup/system/html`date +%m%d`.tgz html/ &
