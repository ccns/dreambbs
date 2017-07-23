#!/bin/bash
date=$(date +%Y%m%d)
cd /home/bbs/gem
tar -zcpf /var/backup/gem@/gem@$date.tar.gz @
