#!/bin/sh
# 使用此 script 前，請先確認已經修改了下面的相關設定
#                                        by statue.bbs@bbs.yzu.edu.tw

# 請在這邊修改成自己的設定
ip="140.138.2.235"
boardname="元智大學 風之塔"
nickname="風之塔"
myhostname="bbs.yzu.edu.tw"
sysopnick="風之塔塔主"
# PS. 以上設定其中皆不能有空白，如 "元智大學 風之塔" 這樣是不合格的

echo "你所設定的 IP 是 $ip .";
echo "你所設定的 BOARDNAME 是 $boardname ."
echo "你所設定的 NICKNAME 是 $nickname ."
echo "你所設定的 MYHOSTNAME 是 $myhostname ."
echo "你所設定的 SYSOPNICK 是 $sysopnick ."

# 目前是否為 FAKE 狀態
FAKE=1

if [ $FAKE = 1 ]; then
  echo "目前是測試狀態 !"
fi

#sysopnick=`grep SYSOPNICK src/include/config.h | 
#awk 'BEGIN {FS="\""}; {print $2}'`


echo "將修改的檔案如下: "
filelist="etc/approved etc/confirm etc/e-mail etc/justified etc/justify \
etc/mail86 etc/mquota etc/newuser etc/summer.mail etc/valid \
src/innbsd/Makefile src/innbbsd/innbbsd.h src/include/global.h bmtad/bmtad.c";
echo $filelist


# 轉換程式碼
if [ $FAKE = 0 ]; then

echo "進行轉換中: "
for i in $filelist
do
cat $i | sed 's/元智大學風之塔/'$boardname'/g' > $i.sed;
cp $i.sed $i
rm $i.sed
cat $i | sed 's/風之塔塔主/'$sysopnick'/g' > $i.sed;
cp $i.sed $i
rm $i.sed
cat $i | sed 's/風之塔/'$nickname'/g' > $i.sed;
cp $i.sed $i
rm $i.sed
cat $i | sed 's/bbs.yzu.edu.tw/'$myhostname'/g' > $i.sed;
cp $i.sed $i
rm $i.sed
cat $i | sed 's/140.138.2.235/'$ip'/g' > $i.sed;
cp $i.sed $i
rm $i.sed
done

fi
