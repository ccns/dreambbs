#!/bin/sh
# �ϥΦ� script �e�A�Х��T�{�w�g�ק�F�U���������]�w
#                                        by statue.bbs@bbs.yzu.edu.tw

# �Цb�o��ק令�ۤv���]�w
ip="140.138.2.235"
boardname="�����j�� ������"
nickname="������"
myhostname="bbs.yzu.edu.tw"
sysopnick="�������D"
# PS. �H�W�]�w�䤤�Ҥ��঳�ťաA�p "�����j�� ������" �o�ˬO���X�檺

echo "�A�ҳ]�w�� IP �O $ip .";
echo "�A�ҳ]�w�� BOARDNAME �O $boardname ."
echo "�A�ҳ]�w�� NICKNAME �O $nickname ."
echo "�A�ҳ]�w�� MYHOSTNAME �O $myhostname ."
echo "�A�ҳ]�w�� SYSOPNICK �O $sysopnick ."

# �ثe�O�_�� FAKE ���A
FAKE=1

if [ $FAKE = 1 ]; then
  echo "�ثe�O���ժ��A !"
fi

#sysopnick=`grep SYSOPNICK src/include/config.h | 
#awk 'BEGIN {FS="\""}; {print $2}'`


echo "�N�ק諸�ɮצp�U: "
filelist="etc/approved etc/confirm etc/e-mail etc/justified etc/justify \
etc/mail86 etc/mquota etc/newuser etc/summer.mail etc/valid \
src/innbsd/Makefile src/innbbsd/innbbsd.h src/include/global.h bmtad/bmtad.c";
echo $filelist


# �ഫ�{���X
if [ $FAKE = 0 ]; then

echo "�i���ഫ��: "
for i in $filelist
do
cat $i | sed 's/�����j�ǭ�����/'$boardname'/g' > $i.sed;
cp $i.sed $i
rm $i.sed
cat $i | sed 's/�������D/'$sysopnick'/g' > $i.sed;
cp $i.sed $i
rm $i.sed
cat $i | sed 's/������/'$nickname'/g' > $i.sed;
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
