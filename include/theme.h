/*-------------------------------------------------------*/
/* theme.h      ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* target : custom theme                                 */
/* create : 09/07/28                                     */
/* update : 17/07/26                                     */
/*-------------------------------------------------------*/


#ifndef THEME_H
#define THEME_H

/* ----------------------------------------------------- */
/* �Ȧs��                                                */
/* ----------------------------------------------------- */

#define NEWMAILMSG              "\x1b[41;37;5m �l�t�Ķi�ӤF! \x1b[0;1;33;44m"
#define NEWPASSMSG              "\x1b[42;37;5m ���쯫���d��! \x1b[0;1;33;44m"

#define ICON_GAMBLED_BRD        "\x1b[1;31m��\x1b[m"    /* �|���L�����ݪO */
#define ICON_VOTED_BRD          "\x1b[1;33m��\x1b[m"    /* �|��벼�����ݪO */

//�U�@������ɶ�����
#define NEXTPUSHTIME 15

/* ----------------------------------------------------- */
/* ���C��w�q�A�H�Q�����ק�                            */
/* ----------------------------------------------------- */

#define ANSIRESET       "\x1b[m"        /* �פ��C�� */

#define COLOR0          "\x1b[44m"      /* header ������ */
#define COLOR1          "\x1b[34;46m"   /* footer/feeter ���e�q�C�� */
#define COLOR2          "\x1b[31;47m"   /* footer/feeter ����q�C�� */
#define COLOR3          "\x1b[30;47m"   /* neck ���C�� */
#define COLOR4          "\x1b[1;44m"    /* ���� ���C�� */
#define COLOR5          "\x1b[34;47m"   /* more ���Y�����D�C�� */
#define COLOR6          "\x1b[37;44m"   /* more ���Y�����e�C�� */
#define COLOR7          "\x1b[0m"       /* �@�̦b�u�W���C�� */


/* ----------------------------------------------------- */
/* �ϥΪ̦W���C��                                        */
/* ----------------------------------------------------- */

#define COLOR_PAL       "\x1b[1;32m"
#define COLOR_BAD       "\x1b[1;31m"
#define COLOR_CLOAK     "\x1b[1;35m"
#define COLOR_BOTH      "\x1b[1;36m"
#define COLOR_OPAL      "\x1b[1;33m"
#define COLOR_BOARDPAL  "\x1b[36m"

/* ----------------------------------------------------- */
/* ����m                                              */
/* ----------------------------------------------------- */



/* ----------------------------------------------------- */
/* �T���r��G*_neck() �ɪ� necker ����X�өw�q�b�o       */
/* ----------------------------------------------------- */

#define NECKBOARD  "  [��]�D��� [��]�\\Ū [����]��� [c]�g�� [y]���J [/]�j�M [s]�ݪO [h]����\n" \
            COLOR3 "  %-7s��  �O            %-*s�H�� �O    �D      \x1b[m"

/*#define NECKGEM1 \
                   "  [��]���} [��]�s�� [f]�Ҧ� [C]�Ȧs [F]��H [Z]�U�� [h]���� "
*/

#define NECKGEM1 \
                   "  [��]���} [��]�s�� [f]�Ҧ� [C]�Ȧs [h]���� "
                   /*r2.20170802: disable some tips in case of confusion*/

#define NECKGEM2 \
            COLOR0 "  �s��     �D              �D%*s                             [�s      ��] [��  ��]\x1b[m"

#define NECKPOST \
                   "  [��]���} [��]�\\Ū [^P]�o�� [b]�Ƨѿ� [d]�R�� [V]�벼 [TAB]��ذ� [h]����\n" \
            COLOR3 "  �s��     �� �� �@  ��       ��  ��  ��  �D%*s                  %16s \x1b[m"

#define NECKMAIL \
                   "  [��]���} [��]Ū�H [d]�R�� [R]�^�H [s]�H�H [x]��K [y]�s�զ^�H [h]����\n" \
            COLOR0 "  �s��   �� �� �@ ��        �H  ��  ��  �D%*s                                     \x1b[m"

#define NECKVOTE \
                   "  [��]���} [v]�벼 [R]���G [E]�ק� [^P]�|�� [^Q]�d��/����/��� [h]����\n" \
            COLOR0 "  �s��  �}����  �D��H       ��  ��  �v  ��%*s                                    \x1b[m"

#define NECKINNBBS "  [��]���} [^P]�s�W [d]�R�� [E]�s�� [/]�j�M [Enter]�Բ�\n" \
            COLOR3 "  �s��              ��         �e%*s                                              \x1b[m"

// IID.190324

/* ulist_neck() �� xpost_head() ���Ĥ@�����S�O�A���b���w�q */

#define NECK_BANMAIL \
                   "  [��]���} ^P)�s�W c)�ק� d)�R�� S)���� [h]elp\n" \
            COLOR3 "  �s��  �ϥβv ��s�� ��  ��  ��  �H  ��  �e%*s                                   \x1b[m"

#define NECK_MYFAVORITE \
                   "  [��]�D��� [��]�\\Ū [a]�s�W [d]�R�� [c]�g�� [/]�j�M [s]�ݪO [h]����\n" \
            COLOR0 "  %-7s��  �O            %-*s�H�� �O    �D      \x1b[m"

#define NECK_XPOST "\n" /*r2.20181219: this part may need to be refined*/ \
            COLOR3 "  �s��    �� �� �@  ��       ��  ��  ��  �D%*s                                    \x1b[m"

#define NECK_ULIST "\n" \
            COLOR3 "  No.  �N��         %-*s%-*s   PM %-14s���m\x1b[m"

#define NECK_PAL   "  [��]���} a)�s�W c)�ק� d)�R�� m)�H�H s)��z [/?]�j�M [q]�d�� [h]elp\n" \
            COLOR3 "  �s��    �N ��         ��       ��%*s                                            \x1b[m"

#define NECK_BMW   "  [��]���}  [d]�R��  [m]�H�H  [w]�ְT  [s]��s  [��]�d��  [h]elp\n" \
            COLOR3 "  �s�� �N ��        ��       �e%*s                                                \x1b[m"

#define NECK_BMWTIME \
                   "  [��]���}  [d]�R��  [m]�H�H  [w]�ְT  [s]��s  [��]�d��  [h]elp\n" \
            COLOR3 "  �s�� �� �� �N ��        ��       �e%*s                                          \x1b[m"

#define NECK_BANMSG \
                   "  [��]���} a)�s�W c)�ק� d)�R�� m)�H�H s)��z [q]�d�� [h]elp\n" \
            COLOR3 "  �s��    �N ��         �y       �z%*s                                            \x1b[m"

// so/*

#define NECK_ADMIN \
                   "  [��]���} ^P)�s�W c)�ק� d)�R�� s)���� [h]elp\n" \
            COLOR3 "  �s��     ��  ��  �W  ��%*s                                                      \x1b[m"

#define NECK_ALOHA \
                   "  [��]���} a)�s�W d)�R�� s)���� f)�ޤJ�n�ͦW�� [h]elp\n" \
            COLOR5 "  �s��     �W �� �q �� �W ��%*s                                                   \x1b[m"

#define NECK_BRDSTAT \
                   "  [��]���} s)���� S)�ثe��T [h]elp\n" \
            COLOR3 "  �s��   ��    �A  �\\Ū����  �o�妸��  ��H����  �׫H����%*s                      \x1b[m"

#define NECK_CHATMENU \
                   "  [��]���} ^P)�s�W c)�ק� d)�R�� s)���� M)�h�� TAB)�ʵ� f)�T�� [h]elp\n" \
            COLOR3 "  �s�� �ʵ�      ����   %-*s                                                       \x1b[m"

#define NECK_CLEANRECOMMEND \
                   "  [��]���} c)�ק�[�����M��] d)�R�� D)�M������ s)���� [h]elp\n" \
            COLOR0 "  �s�� ��    �ϥΪ� �d��%*s                                                  �� ��\x1b[m"

#define NECK_CONTACT \
                   "  [��]���} ^P)�s�W c)�ק� d)�R�� s)���� m)�H�H [h]elp\n" \
            COLOR3 "  �s��     �p  ��  �W  ��     e-mail address%*s                                   \x1b[m"

#define NECK_LISTBRD \
                   "  [��]���} a)�s�W d)�R�� s)���� TAB)�����W�� T)���s�զW�� /)�j�M\n" \
            COLOR3 "  �s��     �ݪO�W��%*s                                                            \x1b[m"

#define NECK_LISTUSR \
                   "  [��]���} a)�s�W d)�R�� s)���� TAB)�����W�� T)���s�զW�� /)�j�M\n" \
            COLOR3 "  �s��     �ϥΪ� ID%*s                                                           \x1b[m"

/*#define NECK_MAILGEM1 \
                   "  [��]���} [��]�s�� [f]�Ҧ� [C]�Ȧs [F]��H [Z]�U�� [h]���� "
*/
#define NECK_MAILGEM1 \
                   "  [��]���} [��]�s�� [f]�Ҧ� [C]�Ȧs [h]���� "
                   /*r2.20170802: disable "Z" option in case of confusion*/

#define NECK_MAILGEM2 \
            COLOR0 "  �s��     �D              �D%*s                             [�s      ��] [��  ��]\x1b[m"

#define NECK_MEMORANDUM \
                   "  [��]���} ^P)�s�W c)�ק� d)�R�� s)���� [h]elp\n" \
            COLOR3 "  �s��  ���      �ɶ�      �u�@�Φ�{%*s                                         \x1b[m"

#define NECK_NBRD  "  [��]���} [��]�\\Ū [^P]�o�� [d]�R�� [j]�[�J�s�p [TAB]��ذ� [h]elp\n" \
            COLOR0 "  �s��   �� �� �|��H       ��  ��  ��  �D%*s                                     \x1b[m"

#define NECK_OBSERVE \
                   "  [��]���} ^P)�s�W c)�ק� d)�R�� S)���� [h]elp\n" \
            COLOR3 "  �s�� �ϥΪ�ID     ����%*s                                                       \x1b[m"

#define NECK_PERSONAL1 \
                   "  [��]���} c)�ק� d)�R�� s)���� TAB)���媩�W/E-mail O)�}�O D)�h�� [h]elp\n" \
            COLOR0 "  �s��   �ӽФH       �ݪO�W��     "

#define NECK_PERSONALEMAIL2 \
                   "E-mail%*s                                   \x1b[m"

#define NECK_PERSONALTITLE2 \
                   "���媩�W%*s                                 \x1b[m"

#define NECK_SHOW  "  [��]���} ^P)�s�W c)�ק� d)�R�� s)���� [h]elp\n" \
            COLOR3 "  �s��     �w�벼�� Email%*s                                                      \x1b[m"

#define NECK_VIOL  "  [��]���} ^P)�s�W c)�ק� d)�R�� f)�j�M [h]elp\n" \
            COLOR3 "  �s�� ���� �ɶ� �T����U Email%*s                                                \x1b[m"

#define NECK_SONG  "  [��]���} [��]�s�� [o]�I�q��ʺA�ݪO [m]�I�q��H�c [q]�d�߳Ѿl���� [h]���� \n" \
            COLOR0 "  �s��     �D              �D%*s                             [�s      ��] [��  ��]\x1b[m"

// cache.101119

#define NECKER_VOTE     "  [��]���} [R]���G [^P]�|�� [E]�ק� [V]�w�� [^Q]��� [o]�W�� [h]����\n" \

#define NECKER_VOTEALL  "  [��/��]�W�U [PgUp/PgDn]�W�U�� [Home/End]���� [��]�벼 [��][q]���}\n" \


/* ----------------------------------------------------- */
/* �T���r��Gmore() �ɪ� footer ����X�өw�q�b�o         */
/* ----------------------------------------------------- */


/* itoc.010914.����: ��@�g�A�ҥH�s FOOTER�A���O 79 char */

#define FOOTER_POST     \
    COLOR1 " �峹��Ū " COLOR2 " (ry)�^�� (=\\[]<>-+;'`)�D�D (|?QA)�j�M���D�@�� (kj)�W�U�g (C)�Ȧs    "

#define FOOTER_MORE     \
    COLOR1 " �s�� P.%d (%d%%) " COLOR2 " (h)���� [PgUp][PgDn][0][$]���� (/n)�j�M (C)�Ȧs (��q)����  "


#define FOOTER_TALK \
    COLOR1 " ��ͼҦ� " COLOR2 " (^A)\x1b[30m�﫳�Ҧ� \x1b[31m(^B)\x1b[30m�H�ѼҦ� \x1b[31m(^C, ^D)\x1b[30m������� \x1b[31m(^Z)\x1b[30m�ֱ��C�� \x1b[31m(^G)\x1b[30m�͹�    "


#define FOOTER_VEDIT_BIFF \
               "%s\x1b[m" COLOR2 "  (Ctrl-Z)\x1b[30m �ާ@����  \x1b[31m(^w, ^x)\x1b[30m �ɮ׳B�z ��%s�x%s��%5d:%3d%*s     \x1b[m"

#define FOOTER_VEDIT \
    COLOR1 "  �s��峹  " COLOR2 "  (Ctrl-Z)\x1b[30m �ާ@����  \x1b[31m(^w, ^x)\x1b[30m �ɮ׳B�z ��%s�x%s��%5d:%3d%*s     \x1b[m"


/*#define FOOTER_BMW_REPLY \
    COLOR1 " ���T�^�� " COLOR2 " (��)\x1b[30m���} \x1b[31m(������)\x1b[30m�s�� \x1b[31m(Enter)\x1b[30m��ܽu�W�ϥΪ̦��� \x1b[31m(��L)\x1b[30m�^��%*s        \x1b[m"
*/
#define FOOTER_BMW_REPLY \
    COLOR1 " ���T�^�� " COLOR2 " (�� Enter)\x1b[30m���} \x1b[31m(������)\x1b[30m�s�� \x1b[31m(��L)\x1b[30m�^��%*s                            \x1b[m"


#define MSG_POST \
    COLOR1 " �峹��Ū " COLOR2 " (y)\x1b[30m�^�� \x1b[31m(=\\[]<>-+;'`jk)\x1b[30m�����D�D \x1b[31m(/?)\x1b[30m�j�M���D \x1b[31m(aA)\x1b[30m�j�M�@��           "

#define MSG_MAILER \
    COLOR1 " �������� " COLOR2 " (r)\x1b[30m�^�H \x1b[31m(x)\x1b[30m��F \x1b[31m(y)\x1b[30m�s�զ^�H \x1b[31m(d)\x1b[30m�R�� \x1b[31m[m]\x1b[30m�аO                         "

#define MSG_GEM \
    COLOR1 " �\\Ū��� " COLOR2 " (=\\[]<>-+jk)\x1b[30m�����D�D \x1b[31m(/?aA)\x1b[30m�j�M���D�@�� \x1b[31m(����)\x1b[30m�W�U \x1b[31m(��)\x1b[30m���}         "


/* ----------------------------------------------------- */
/* �T���r��Gxo_foot() �ɪ� feeter ����X�өw�q�b�o      */
/* ----------------------------------------------------- */


/* itoc.010914.����: �C��h�g�A�ҥH�s FEETER�A���O 79 char */

#define FEETER_INNBBS   \
    COLOR1 " ��H�]�w " COLOR2 " (��/��)�W�U (PgUp/PgDn)�W�U�� (Home/End)���� (��)(q)���}            "

#define FEETER_VOTEALL  \
    COLOR1 " �벼���� " COLOR2 " (��/��)�W�U (PgUp/PgDn)�W�U�� (Home/End)���� (��)�벼 (��)(q)���}   "

/* ----------------------------------------------------- */
/* ��ñ�G���x�ӷ�ñ�W                                    */
/* ----------------------------------------------------- */

#ifndef ORIGIN_TAG
#define ORIGIN_TAG \
    "--\n" \
    "\x1b[1;44;32m  ��\x1b[0;32;44m��   \x1b[1;37m�_�_     \x1b[30m�i�v�� �i�v�i �i�v�v �i�v�i �i�g�p �i   �i�v�i �i���i �i�v\x1b[31m�� \x1b[m\n" \
    "\x1b[1;32;44m��\x1b[42m��\x1b[0;44;32m�i��\x1b[1m��\x1b[0;32;44m�� \x1b[1;37m�_�_ \x1b[0;37;44m�i  �i �i�b�� �i�b�b �i�b�i �p�p�p �i   �i�b�i �i���i �i  �i \x1b[m\n" \
    "\x1b[1;36;42m�ڤ��j�a �G�G���| \x1b[37m�i�b�� �i  �i �i�b�b �i  �i �p�p�p �i�b �i  �i �i  �i �i�b�� \x1b[m\n" \
    "\x1b[1;32m�� Origin:\x1b[33m <bbs.ccns.ncku.edu.tw>  \x1b[31m�� From:\x1b[36m %-34.34s \x1b[m\n"
#endif  // ORIGIN_TAG

#define MODIFY_TAG    "\x1b[1;32m�� Modify: \x1b[33m<%s> \x1b[m%s"

#define ANONYMOUS_TAG "--\n\x1b[1;32m�� Origin: \x1b[33m%s \x1b[37m<%s> \x1b[m\n\x1b[1;31m�� From: \x1b[36m%s\x1b[m\n"

#define EMAIL_TAG     "\x1b[1;32m�� E-mail: \x1b[36m%s\x1b[m\n"

/* ----------------------------------------------------- */
/* ��L�T���r��                                          */
/* ----------------------------------------------------- */

//for M3_USE_PFTERM
#define FILEDFG            (0)
#define FILEDBG            (7)

#endif                          /* THEME_H */
