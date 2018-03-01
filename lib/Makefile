# ------------------------------------------------------ #
#  lib/Makefile		( NTHU CS MapleBBS Ver 3.00 )	 #
# ------------------------------------------------------ #
#  author : opus.bbs@bbs.cs.nthu.edu.tw		 	 #
#  target : Makefile for MapleBBS library routines	 #
#  create : 95/03/29				 	 #
#  update : 95/12/15					 #
# ------------------------------------------------------ #


CC	= gcc
RANLIB	= ranlib
CPROTO	= `which cproto` -E"gcc -pipe -E" -I../include
CFLAGS	= -O2 -pipe -fomit-frame-pointer -Wunused -I../include -m32


# ------------------------------------------------------ #
# 下列的 make rules 不需修改				 #
# ------------------------------------------------------ #

HDR =	dao.h dao.p

SRC =	\
	is_alnum.c is_alpha.c is_fname.c is_fpath.c not_addr.c \
	\
	dns.c dns_addr.c dns_name.c dns_smtp.c dns_ident.c dns_open.c \
	\
	str_add.c str_cat.c str_cmp.c str_decode.c str_dup.c str_folder.c \
	str_fpath.c str_from.c str_has.c str_hash.c str_len.c str_lower.c \
	str_ncmp.c str_ncpy.c str_ndup.c str_passwd.c str_pat.c str_rev.c \
	str_rle.c str_stamp.c str_str.c str_tail.c str_time.c str_hash2.c \
	str_trim.c str_ttl.c str_xor.c url_encode.c str_ansi.c str_lcpy.c str_lcat.c \
	str_sub.c str_lowest.c strlcat.c strlcpy.c \
	\
	archiv32.c archiv32m.c chrono32.c hash32.c radix32.c \
	\
	f_cat.c f_cp.c f_img.c f_ln.c f_map.c f_mode.c f_mv.c \
	f_new.c f_open.c f_path.c f_rm.c f_suck.c mak_dirs.c f_lock.c \
	\
	rec_add.c rec_num.c rec_del.c rec_get.c rec_loc.c\
	rec_ins.c rec_mov.c rec_put.c rec_bot.c rec_ref.c rec_sync.c\
	\
	hdr_fpath.c hdr_stamp.c \
	\
	shm.c \
	\
	splay.c \
	\
	acl_addr.c acl_has.c xsort.c xwrite.c dl_lib.c attr_lib.c rfc2047.c

OBJ =	\
	is_alnum.o is_alpha.o is_fname.o is_fpath.o not_addr.o \
	\
	dns.o dns_addr.o dns_name.o dns_smtp.o dns_ident.o dns_open.o \
	\
	str_add.o str_cat.o str_cmp.o str_decode.o str_dup.o str_folder.o \
	str_fpath.o str_from.o str_has.o str_hash.o str_len.o str_lower.o \
	str_ncmp.o str_ncpy.o str_ndup.o str_passwd.o str_pat.o str_rev.o \
	str_rle.o str_stamp.o str_str.o str_tail.o str_time.o str_hash2.o \
	str_trim.o str_ttl.o str_xor.o url_encode.o str_ansi.o str_lcpy.o str_lcat.o \
	str_sub.o str_lowest.o \
	\
	archiv32.o archiv32m.o chrono32.o hash32.o radix32.o \
	\
	f_cat.o f_cp.o f_img.o f_ln.o f_map.o f_mode.o f_mv.o \
	f_new.o f_open.o f_path.o f_rm.o f_suck.o mak_dirs.o f_lock.o \
	\
	rec_add.o rec_num.o rec_del.o rec_get.o rec_loc.o\
	rec_ins.o rec_mov.o rec_put.o rec_bot.o rec_ref.o rec_sync.o\
	\
	hdr_fpath.o hdr_stamp.o \
	\
	shm.o \
	\
	splay.o \
	\
	acl_addr.o acl_has.o xsort.o xwrite.o dl_lib.o attr_lib.o rfc2047.o


.c.o:   ;   $(CC) $(CFLAGS) -c $*.c


all:	libdao.a


dao.p:	$(SRC)
	$(CPROTO) $> | sed '/querybuf/d' > dao.p


libdao.a: $(OBJ)
	ar rv $@ $?
	$(RANLIB) $@


clean:
	rm -fr $(GARBAGE) $(OBJ) libdao.a *.bak *.BAK *~ DEADJOE


tags:
	ctags $(SRC) ../include/*.h
