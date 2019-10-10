#!/bin/sh
# 排行榜程式執行
if [ "${BBSHOME}" = "" ]; then BBSHOME="/home/bbs"; fi
"${BBSHOME}/bin/topusr" > "${BBSHOME}/gem/@/@pop.new"
mv "${BBSHOME}/gem/@/@pop.new" "${BBSHOME}/gem/@/@pop"
