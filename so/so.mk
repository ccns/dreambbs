# -------------------------------------------------------#
#  so/so.mk     ( NCKU CCNS WindTop-DreamBBS 2.0 )       #
# -------------------------------------------------------#
#  Author: opus.bbs@bbs.cs.nthu.edu.tw                   #
#  Target: Shared object target files for DreamBBS       #
#  Create: 95/03/29 (so/Makefile)                        #
#  Update: 2019/05/07 (split from so/Makefile)           #
#        : by Wei-Cheng Yeh (IID) <iid@ccns.ncku.edu.tw> #
# -------------------------------------------------------#

HDR	= bbs.h config.h global.h modes.h perm.h struct.h bbs_script.h cppdef.h

SO	= chat.so vote.so xyz.so guessnum.so \
	  admin.so chatmenu.so  mailgem.so\
	  memorandum.so aloha.so newboard.so violate.so song.so same_mail.so\
	  showvote.so list.so mine.so bj.so \
	  pnote.so passwd.so adminutil.so ascii.so\
	  classtable2.so observe.so pip.so brdstat.so personal.so \
	  cleanrecommend.so shop.so bank.so innbbs.so contact.so banmail.so
