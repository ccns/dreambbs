#!/bin/sh
echo -n "�����\Ū�ϥΦ��ơG" > run/spss.log
wc run/brd_usies | awk '{print $1}' >> run/spss.log
echo -n "MailService �ϥΦ��ơG" >> run/spss.log
wc run/mailservice.log | awk '{print $1}' >> run/spss.log
echo -n "�H�H�� Internet �ϥΦ��ơG" >> run/spss.log
value=`wc run/mail.log | awk '{print $1}'`
#wc run/mail.log | awk '{print $1}' >> run/spss.log
echo `expr $value / 3` >> run/spss.log
echo -n "�I�q�ϥΦ��ơG" >> run/spss.log
wc run/ordersongs.log | awk '{print $1}' >> run/spss.log
echo -n "�ΦW�O�ϥΦ��ơG" >> run/spss.log
wc run/anonymous.log | awk '{print $1}' >> run/spss.log
echo -n "�������d�����ϥΦ��ơG" >> run/spss.log
wc run/pip.log | awk '{print $1}' >> run/spss.log
echo -n "POP3D�s�u���ơG" >>  run/spss.log
grep CONN run/pop3.log | wc | awk '{print $1}' >> run/spss.log
echo -n "�Ĳ��W���d���O�ϥΦ��ơG" >> run/spss.log
grep "�W" run/note.all | wc | awk '{print $1}' >> run/spss.log
echo -n "�j���p�Ҭd�]�ϥΦ��ơG" >> run/spss.log
wc run/ueequery.log | awk '{print $1}' >> run/spss.log
echo -n "�ڪ��̷R�ϥΦ��ơG" >> run/spss.log
wc run/favorite.log | awk '{print $1}' >> run/spss.log
echo -n "bbsmail mailpost �ϥΦ��ơG" >> run/spss.log
wc run/bbsmailpost.log | awk '{print $1}' >> run/spss.log

echo "" >> run/spss.log
