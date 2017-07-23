#!/bin/sh
echo -n "綷弄ㄏノΩ计" > run/spss.log
wc run/brd_usies | awk '{print $1}' >> run/spss.log
echo -n "MailService ㄏノΩ计" >> run/spss.log
wc run/mailservice.log | awk '{print $1}' >> run/spss.log
echo -n "盚獺 Internet ㄏノΩ计" >> run/spss.log
value=`wc run/mail.log | awk '{print $1}'`
#wc run/mail.log | awk '{print $1}' >> run/spss.log
echo `expr $value / 3` >> run/spss.log
echo -n "翴簈ㄏノΩ计" >> run/spss.log
wc run/ordersongs.log | awk '{print $1}' >> run/spss.log
echo -n "拔狾ㄏノΩ计" >> run/spss.log
wc run/anonymous.log | awk '{print $1}' >> run/spss.log
echo -n "ぇ娥胐蔓ㄏノΩ计" >> run/spss.log
wc run/pip.log | awk '{print $1}' >> run/spss.log
echo -n "POP3D硈絬Ω计" >>  run/spss.log
grep CONN run/pop3.log | wc | awk '{print $1}' >> run/spss.log
echo -n "荒并璚欢痙ē狾ㄏノΩ计" >> run/spss.log
grep "" run/note.all | wc | awk '{print $1}' >> run/spss.log
echo -n "厩羛σ琩篯ㄏノΩ计" >> run/spss.log
wc run/ueequery.log | awk '{print $1}' >> run/spss.log
echo -n "и程稲ㄏノΩ计" >> run/spss.log
wc run/favorite.log | awk '{print $1}' >> run/spss.log
echo -n "bbsmail mailpost ㄏノΩ计" >> run/spss.log
wc run/bbsmailpost.log | awk '{print $1}' >> run/spss.log

echo "" >> run/spss.log
