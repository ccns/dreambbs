/*-------------------------------------------------------*/
/* more.c       ( NTHU CS MapleBBS Ver 3.00 )            */
/*-------------------------------------------------------*/
/* target : simple & beautiful ANSI/Chinese browser      */
/* create : 95/03/29                                     */
/* update : 97/03/29                                     */
/*-------------------------------------------------------*/


#include "bbs.h"


/* ----------------------------------------------------- */
/* buffered file read                                    */
/* ----------------------------------------------------- */


#define MORE_BUFSIZE    4096


#ifndef M3_USE_PMORE
static int more_width;  /* more screen ���e�� */
#endif

static char more_pool[MORE_BUFSIZE];
static int more_base;           /* more_pool[more_base ~ more_base+more_size] ���� */
static int more_size;


/* ----------------------------------------------------- */
/* mget �\Ū��r�ɡFmread �\Ū�G�i����                   */
/* ----------------------------------------------------- */


/* itoc.041226.����: mgets() �M more_line() ���@�˪�������
   1. mgets ������ more_pool ���Ŷ��Fmore_line �h�O�|��ȼg�J�@�� buffer
   2. mgets ���|�۰��_��Fmore_line �h�O�|�۰��_��b more_width
   �ҥH mgets �O���Φb�@�Ǩt���ɳB�z�άO edit.c�A�� more_line �u�Φb more()
 */


char *
mgets(
    int fd)
{
    char *pool, *base, *head, *tail;
    int ch;

    if (fd < 0)
    {
        more_base = more_size = 0;
        return NULL;
    }

    pool = more_pool;
    base = pool + more_base;
    tail = pool + more_size;
    head = base;

    for (;;)
    {
        if (head >= tail)
        {
            /* IID.20191222:
             *    Read little and then run out of chars in the middle of a long line
             *       => head - base > base - pool => overlap */
            if ((ch = head - base))
                memmove(pool, base, ch);

            head = pool + ch;
            ch = read(fd, head, MORE_BUFSIZE - ch);

            if (ch <= 0)
                return NULL;

            base = pool;
            tail = head + ch;
            more_size = tail - pool;
        }

        ch = *head;

        if (ch == '\n')
        {
            *head++ = '\0';
            more_base = head - pool;
            return base;
        }

        head++;
    }
}


/* use mgets(-1) to reset */


void *
mread(
    int fd, int len)
{
    char *pool;
    int base, size;

    base = more_base;
    size = more_size;
    pool = more_pool;

    if (size < len)
    {
        /* IID.20191222:
         *    Read little and then run out of chars for large `len`
         *       => size > base => overlap */
        if (size)
            memmove(pool, pool + base, size);

        base = read(fd, pool + size, MORE_BUFSIZE - size);

        if (base <= 0)
            return NULL;

        size += base;
        base = 0;
    }

    more_base = base + len;
    more_size = size - len;

    return pool + base;
}


/* ----------------------------------------------------- */
/* more �\Ū��r��                                       */
/* ----------------------------------------------------- */


#ifndef M3_USE_PMORE
#define STR_ANSICODE    "[0123456789;"


static char *fimage;           /* file image begin */
static const char *fend;       /* file image end */
static const char *foff;       /* �ثeŪ����� */


static int
more_line(
    char *buf)
{
    int ch, len, bytes, in_ansi, in_dbcs;

    len = bytes = in_ansi = in_dbcs = 0;

    for (;;)
    {
        if (foff >= fend)
            break;

        ch = (unsigned char) *foff;

        /* weiyu.040802: �p�G�o�X�O����r�����X�A���O�u�ѤU�@�X���Ŷ��i�H�L�A���򤣭n�L�o�X */
        if (in_dbcs || IS_DBCS_HI(ch))
            in_dbcs ^= 1;
        if (in_dbcs && (len >= more_width - 1 || bytes >= ANSILINESIZE - 2))
            break;

        foff++;
        bytes++;

        if (ch == '\n')
            break;

        if (ch == KEY_ESC)
        {
            in_ansi = 1;
        }
        else if (in_ansi)
        {
            if (!strchr(STR_ANSICODE, ch))
                in_ansi = 0;
        }
        else if (isprint2(ch))
        {
            len++;
        }
        else
        {
            ch = ' ';           /* �L�X���Ӫ��������ť� */
            len++;
        }

        *buf++ = ch;

        /* �Y���t����X�����פw�F more_width �r�A�Χt����X�����פw�F ANSILINESIZE-1�A�������}�j�� */
        if (len >= more_width || bytes >= ANSILINESIZE - 1)
        {
            /* itoc.031123: �p�G�O����X�A�Y�Ϥ��t����X�����פw�F more_width �F�A�٥i�H�~��Y */
            if ((in_ansi || (foff < fend && *foff == KEY_ESC)) && bytes < ANSILINESIZE - 1)
                continue;

            /* itoc.031123: �A�ˬd�U�@�Ӧr�O���O '\n'�A�קK��n�O more_width �� ANSILINELEN-1 �ɡA�|�h���@�C�ť� */
            if (foff < fend && *foff == '\n')
            {
                foff++;
                bytes++;
            }
            break;
        }
    }

    *buf = '\0';

    return bytes;
}


static void
outs_line(                      /* �L�X�@�뤺�e */
    const char *str)
{
    bool ansi = false;

    /* ���B�z�ޥΪ� & �ި� */

    const int ch1 = str[0];
    if ((ch1 == QUOTE_CHAR1 || ch1 == QUOTE_CHAR2) && str[1] == ' ')    /* �ި� */
    {
        const int ch3 = str[2];
        outs((ch3 == QUOTE_CHAR1 || ch3 == QUOTE_CHAR2) ? "\x1b[33m" : "\x1b[36m");     /* �ޥΤ@�h/�G�h���P�C�� */
        ansi = true;
    }
    else if (!strncmp(str, "��", sizeof("��" - 1))) /* �� �ި��� */
    {
        outs("\x1b[1;36m");
        ansi = true;
    }

    /* �L�X���e */

    if (!hunt[0])
    {
        outx(str);
    }
    else
    {
        char buf[ANSILINESIZE];
        char *ptr2 = buf;
        const int len = strlen(hunt);

        for (;;)
        {
            const char *const ptr1 = str_casestr_dbcs(str, hunt);
            if (!ptr1)
            {
                str_scpy(ptr2, str, buf + sizeof(buf) - ptr2);
                break;
            }

            if (ptr2 + (ptr1 - str) + (len + 7) >= buf + sizeof(buf) - 1)       /* buf �Ŷ����� */
                break;

            str_sncpy(ptr2, str, buf + sizeof(buf) - ptr2, ptr1 - str);
            ptr2 += ptr1 - str;
            sprintf(ptr2, "\x1b[7m%.*s\x1b[m", len, ptr1);
            ptr2 += len + 7;
            str = ptr1 + len;
        }

        outx(buf);
    }

    if (ansi)
        outs(str_ransi);
}


static void
outs_header(    /* �L�X���Y */
    char *str,
    int header_len)
{
    static const char header1[LINE_HEADER][LEN_AUTHOR1] = {"�@��",   "���D",   "�ɶ�", "���|"};
    static const char header2[LINE_HEADER][LEN_AUTHOR2] = {"�o�H�H", "��  �D", "�o�H��", "��H��"};
    int i;
    char *ptr, *word;

    /* �B�z���Y */

    if ((header_len == LEN_AUTHOR1 && !memcmp(str, header1[0], LEN_AUTHOR1 - 1)) ||
        (header_len == LEN_AUTHOR2 && !memcmp(str, header2[0], LEN_AUTHOR2 - 1)))
    {
        /* �@��/�ݪO ���Y���G��A�S�O�B�z */
        word = str + header_len;
        if ((ptr = strstr(word, str_post1)) || (ptr = strstr(word, str_post2)))
        {
            ptr[-1] = ptr[4] = '\0';
            prints(COLOR5 " %s " COLOR6 "%-*.*s" COLOR5 " %s " COLOR6 "%-13s\x1b[m",
                header1[0], d_cols + 54, d_cols + 54, word, ptr, ptr + 5);
        }
        else
        {
            /* �֬ݪO�o�� */
            prints(COLOR5 " %s " COLOR6 "%-*.*s\x1b[m",
                header1[0], d_cols + 73, d_cols + 73, word);
        }
        return;
    }

    for (i = 1; i < LINE_HEADER; i++)
    {
        if ((header_len == LEN_AUTHOR1 && !memcmp(str, header1[i], LEN_AUTHOR1 - 1)) ||
            (header_len == LEN_AUTHOR2 && !memcmp(str, header2[i], LEN_AUTHOR2 - 1)))
        {
            /* ��L���Y���u���@�� */
            word = str + header_len;
            prints(COLOR5 " %s " COLOR6 "%-*.*s\x1b[m",
                header1[i], d_cols + 73, d_cols + 73, word);
            return;
        }
    }

    /* �p�G���O���Y�A�N��@���r�L�X */
    outs_line(str);
}

static inline void
outs_footer(
    char *buf,
    int lino,
    int fsize)
{
    int i;

    /* P.1 �� (PAGE_SCROLL + 1) �C�A��L Page ���O PAGE_SCROLL �C */

    /* prints(FOOTER_MORE, (lino - 2) / PAGE_SCROLL + 1, ((foff - fimage) * 100) / fsize); */

    /* itoc.010821: ���F�M FOOTER ��� */
    sprintf(buf, FOOTER_MORE, (lino - 2) / PAGE_SCROLL + 1, ((foff - fimage) * 100) / fsize);
    outs(buf);

    for (i = b_cols + sizeof(COLOR1) + sizeof(COLOR2) - strlen(buf); i > 3; i--)
    {
        /* �񺡳̫᪺�ť��C��A�̫�d�@�Ӫť� */
        outc(' ');
    }
    outs(str_ransi);
}


#ifdef SLIDE_SHOW
static int slideshow;           /* !=0: ���� movie ���t�� */

static int
more_slideshow(void)
{
    int ch;

    if (!slideshow)
    {
        ch = vkey();

        if (ch == '@')
        {
            slideshow = vans("�п�ܩ�M���t�� 1(�̺C)��9(�̧�)�H���񤤫����N��i�����G") - '0';
            if (slideshow < 1 || slideshow > 9)
                slideshow = 5;

            ch = KEY_PGDN;
        }
    }
    else
    {
        struct timeval tv[9] =
        {
            {4, 0}, {3, 0}, {2, 0}, {1, 500000}, {1, 0},
            {0, 800000}, {0, 600000}, {0, 400000}, {0, 200000}
        };

        refresh();
        ch = 1;
        if (select(1, (fd_set *) &ch, NULL, NULL, tv + slideshow - 1) > 0)
        {
            /* �Y���񤤫����N��A�h����� */
            slideshow = 0;
            ch = vkey();
        }
        else
        {
            ch = KEY_PGDN;
        }
    }

    return ch;
}
#endif  /* #ifdef SLIDE_SHOW */


#define END_MASK        0x200   /* �� KEY_END ���F�̫�@�� */

#define HUNT_MASK       0x400
#define HUNT_NEXT       0x001   /* �� n �j�M�U�@�� */
#define HUNT_FOUND      0x002   /* �� / �}�l�j�M�A�B�w�g��� match ���r�� */
#define HUNT_START      0x004   /* �� / �}�l�j�M�A�B�|����� match ���r�� */

#define MAXBLOCK        256U    /* �O���X�� block �� offset�C�i�[�t MAXBLOCK*32 �C�H��������b�W��/½�ɪ��t�� */
#endif  /* #ifndef M3_USE_PMORE */

/* Thor.990204: �Ǧ^�� -1 ���L�kshow�X
                        0 ������show��
                       >0 ������show�A���_�ҫ���key */
int
more(
    const char *fpath,
    const char *footer)
{
#ifndef M3_USE_PMORE
    char buf[ANSILINESIZE];
    int i;

    const char *headend;                /* ���Y���� */

    int shift;                          /* �ٻݭn���U���ʴX�C */
    int lino;                           /* �ثe line number */
    int header_len;                     /* ���Y�����סA�P�ɤ]�O����/���~�H���ϧO */
    int key;                            /* ���� */
#endif
    int cmd;                            /* ���_�ɩҫ����� */

#ifndef M3_USE_PMORE
    int fsize;                          /* �ɮפj�p */
    static off_t block[MAXBLOCK];       /* �C 32 �C���@�� block�A�O���� block �� offset */
#endif

#ifdef M3_USE_PMORE
    cmd = pmore(fpath, footer && footer != (char*)-1);
#else

    if (!(fimage = f_img(fpath, &fsize)))
        return -1;

    foff = fimage;
    fend = fimage + fsize;

    /* more_width = b_cols - 1; */      /* itoc.070517.����: �Y�γo�ӡA�C�C�̤j�r�Ʒ|�M header �� footer ��� (�Y�|���d�դG��) */
    /* more_width = b_cols + 1; */      /* itoc.070517.����: �Y�γo�ӡA�C�C�̤j�r�ƻP�ù��P�e */
    more_width = b_cols;  /* IID.20191223: Leave one space */

    /* �����Y�������a�� */
    for (i = 0; i < LINE_HEADER; i++)
    {
        if (!more_line(buf))
            break;

        /* Ū�X�ɮײĤ@�C�A�ӧP�_�����H�٬O���~�H */
        if (i == 0)
        {
            header_len =
                !memcmp(buf, str_author1, LEN_AUTHOR1) ? LEN_AUTHOR1 :  /* �u�@��:�v�����峹 */
                !memcmp(buf, str_author2, LEN_AUTHOR2) ? LEN_AUTHOR2 :  /* �u�o�H�H:�v����H�峹 */
                0;                                                      /* �S�����Y */
        }

        if (!*buf)      /* �Ĥ@�� "\n\n" �O���Y������ */
            break;
    }
    headend = foff;

    /* �k�s */
    foff = fimage;

    lino = cmd = 0;
    block[0] = 0;

#ifdef SLIDE_SHOW
    slideshow = 0;
#endif

    if (hunt[0])                /* �b xxxx_browse() �ШD�j�M�r�� */
    {
        str_lower_dbcs(hunt, hunt);
        shift = HUNT_MASK | HUNT_START;
    }
    else
    {
        shift = b_lines;
    }

    clear();

    while (more_line(buf))
    {
        /* ------------------------------------------------- */
        /* �L�X�@�C����r                                    */
        /* ------------------------------------------------- */

        /* �����e�X�C�~�ݭn�B�z���Y */
        if (foff <= headend)
            outs_header(buf, header_len);
        else
            outs_line(buf);

        outc('\n');

        /* ------------------------------------------------- */
        /* �� shift �ӨM�w�ʧ@                               */
        /* ------------------------------------------------- */

        /* itoc.030303.����: shift �b�����N�q
           >0: �ٻݭn���U���X�C
           <0: �ٻݭn���W���X�C
           =0: �����o���A���ݨϥΪ̫��� */

        if (shift > 0)          /* �٭n�U�� shift �C */
        {
            if (lino >= b_lines)        /* �u���b��i more�A�Ĥ@���L�Ĥ@���ɤ~�i�� lino <= b_lines */
                scroll();

            lino++;

            if ((lino & 31 == 0) && ((i = lino >> 5) < MAXBLOCK))
                block[i] = foff - fimage;


            if (!(shift & (HUNT_MASK | END_MASK)))      /* �@����Ū�� */
            {
                shift--;
            }
            else if (shift & HUNT_MASK)         /* �r��j�M */
            {
                if (shift & HUNT_NEXT)  /* �� n �j�M�U�@�� */
                {
                    /* �@���N����ӦC */
                    if (str_casestr_dbcs(buf, hunt))
                        shift = 0;
                }
                else                    /* �� / �}�l�j�M */
                {
                    /* �Y�b�ĤG���H����A�@���N����ӦC�F
                       �Y�b�Ĥ@�����A��������Ū���Ĥ@���~�ఱ�� */
                    if (shift & HUNT_START && str_casestr_dbcs(buf, hunt))
                        shift ^= HUNT_START | HUNT_FOUND;               /* ���� HUNT_START �å[�W HUNT_FOUND */
                    if (shift & HUNT_FOUND && lino >= b_lines)
                        shift = 0;
                }
            }
        }
        else if (shift < 0)             /* �٭n�W�� -shift �C */
        {
            shift++;

            if (!shift)
            {
                move(b_lines, 0);
                clrtoeol();

                /* �ѤU b_lines+shift �C�O rscroll�Aoffset �h���T��m�F�o�̪� i �O�`�@�n shift ���C�� */
                for (i += b_lines; i > 0; i--)
                    more_line(buf);
            }
        }

        if (foff >= fend)               /* �w�gŪ���������ɮ� */
        {
            /* �խY�O�� End ����̫�@���A���򰱯d�b 100% �Ӥ������F�_�h�@�ߵ��� */
            if (!(shift & END_MASK))
                break;
            shift = 0;
        }

        if (shift)                      /* �ٻݭn�~��Ū��� */
            continue;

        /* ------------------------------------------------- */
        /* �즹�L���һݪ� shift �C�A���U�ӦL�X footer �õ��� */
        /* �ϥΪ̫���                                        */
        /* ------------------------------------------------- */

re_key:

        outs_footer(buf, lino, fsize);

#ifdef SLIDE_SHOW
        key = more_slideshow();
#else
        key = vkey();
#endif

        if (key == ' ' || key == KEY_PGDN || key == KEY_RIGHT || key == Ctrl('F'))
        {
            shift = PAGE_SCROLL;
        }

        else if (key == KEY_DOWN || key == '\n')
        {
            shift = 1;
        }

        else if (key == KEY_PGUP || key == Ctrl('B') || key == KEY_DEL)
        {
            /* itoc.010324: ��F�̶}�l�A�W��������}�A�æ^�� 'k' (keymap[] �w�q�W�@�g) */
            if (lino <= b_lines)
            {
                cmd = 'k';
                break;
            }
            /* �̦h�u��W����@�}�l */
            shift = BMAX(-PAGE_SCROLL, b_lines - lino);
        }

        else if (key == KEY_UP)
        {
            /* itoc.010324: ��F�̶}�l�A�W��������}�A�æ^�� 'k' (keymap[] �w�q�W�@�g) */
            if (lino <= b_lines)
            {
                cmd = 'k';
                break;
            }
            shift = -1;
        }

        else if (key == KEY_END || key == '$')
        {
            shift = END_MASK;
        }

        else if (key == KEY_HOME || key == '0')
        {
            if (lino <= b_lines)        /* �w�g�b�̶}�l�F */
                shift = 0;
            else
                shift = -PAGE_SCROLL - 1;
        }

        else if (key == '/' || key == 'n')
        {
            if (key == 'n' && hunt[0])  /* �p�G�� n �o����J�L�j�M�r��A������P�� / */
            {
                shift = HUNT_MASK | HUNT_NEXT;
            }
            else if (vget(B_LINES_REF, 0, "�j�M�G", hunt, sizeof(hunt), DOECHO))
            {
                str_lower_dbcs(hunt, hunt);
                shift = HUNT_MASK | HUNT_START;
            }
            else                                /* �p�G�����j�M���ܡA��ø footer �Y�i */
            {
                shift = 0;
            }
        }

        else if (key == 'C')    /* Thor.980405: more �ɥi�s�J�Ȧs�� */
        {
            FILE *fp;
            if ((fp = tbf_open(-1)))
            {
                f_suck(fp, fpath);
                fclose(fp);
            }
            shift = 0;          /* ��ø footer */
        }

/* 090924.cache: �ثe�� Windtop ���䴩 */
/*
        else if (key == 'h')
        {
            screen_backup_t old_screen;
            char *tmp_fimage;
            const char *tmp_fend;
            const char *tmp_foff;
            off_t tmp_block[MAXBLOCK];
*/
            /* itoc.060420: xo_help() �|�i�J�ĤG�� more()�A�ҥH�n��Ҧ� static �ŧi�����O���U�� */
/*
            tmp_fimage = fimage;
            tmp_fend = fend;
            tmp_foff = foff;
            memcpy(tmp_block, block, sizeof(tmp_block));

            scr_dump(&old_screen);
            xo_help("post");
            scr_restore_free(&old_screen);

            fimage = tmp_fimage;
            fend = tmp_fend;
            foff = tmp_foff;
            memcpy(block, tmp_block, sizeof(block));

            shift = 0;
        }
*/
        else if ((key == 'H') || (key == 'h') || (key == '?')
#ifdef KEY_F1
                 || (key == KEY_F1)
#endif
        )
        {
            film_out(FILM_MORE, -1);
            shift = 0;
            break;
        }
/* BBS-Lua */
#ifdef M3_USE_BBSLUA
        else if ((key == 'L' || key == 'l') && HAS_PERM(PERM_BBSLUA))
        {
            screen_backup_t old_screen;
            scr_dump(&old_screen);
            bbslua(fpath);
            scr_restore_free(&old_screen);
        }
#endif  /* #ifdef M3_USE_BBSLUA */
#ifdef M3_USE_BBSRUBY
/* 081229.cache: BBSRuby */
        else if (key == '!' && HAS_PERM(PERM_BBSRUBY))
        {
            screen_backup_t old_screen;
            scr_dump(&old_screen);
            run_ruby(fpath);
            scr_restore_free(&old_screen);
        }
#endif  /* #ifdef M3_USE_BBSRUBY */

        else            /* ��L�䳣�O�ϥΪ̤��_ */
        {
            /* itoc.041006: �ϥΪ̤��_������n > 0 (�� KEY_LEFT �O < 0) */
            cmd = key > 0 ? key : 'q';
            break;
        }

        /* ------------------------------------------------- */
        /* �ϥΪ̤w����A�Y break �h���}�j��F�_�h�̷� shift */
        /* ������ (��Y���䪺����) �Ӱ����P���ʧ@            */
        /* ------------------------------------------------- */

        if (shift > 0)                  /* �ǳƤU�� shift �C */
        {
            if (shift < (HUNT_MASK | HUNT_START))       /* �@��U�� */
            {
                /* itoc.041114.����: �ؼЬO�q�X lino-b_lines+1+shift ~ lino+shift �C�����e�G
                   �N�u�n�M footer �Y�i�A��L���N�浹�e���`�ǦL shift �C���{�� */

                move(b_lines, 0);
                clrtoeol();

#if 1
                /* itoc.041116: End ���@�k���M�@��U���i�H�O�����@�˪��A���O�p�G�J��W���峹�ɡA
                   �|�y���e���`�ǦL shift �C���{���N�o�@��½�A������̫�@���A�o�˷|���Ӧh outs_line() �դu�A
                   �ҥH�b���S�O�ˬd�W���峹�ɡA�N���h��̫�@���Ҧb */

                if ((shift & END_MASK) && (fend - foff >= MORE_BUFSIZE))        /* �٦��@��SŪ�L�A�~�S�O�B�z */
                {
                    int totallino = lino;

                    /* ��Ū��̫�@�C�ݬݥ������X�C */
                    while (more_line(buf))
                    {
                        totallino++;
                        if ((totallino & 31 == 0) && ((i = totallino >> 5) < MAXBLOCK))
                            block[i] = foff - fimage;
                    }

                    /* ���첾��W�@�� block ������ */
                    i = BMIN((totallino - b_lines) >> 5, MAXBLOCK - 1);
                    foff = fimage + block[i];
                    i = i * 32;

                    /* �A�q�W�@�� block �����ݦ첾�� totallino-b_lines+1 �C */
                    for (i = totallino - b_lines - i; i > 0; i--)
                        more_line(buf);

                    lino = totallino - b_lines;
                }
#endif  /* #if 1 */
            }
            else
            {
                /* '/' �q�Y�}�l�j�M */
                lino = 0;
                foff = fimage;
                clear();
            }
        }
        else if (shift < 0)                     /* �ǳƤW�� -shift �C */
        {
            if (shift >= -PAGE_SCROLL)  /* �W���ƦC */
            {
                lino += shift;

                /* itoc.041114.����: �ؼЬO�q�X lino-b_lines+1 ~ lino �C�����e�G
                  1. ���q�Y�첾�� lino-b_lines+1 �C
                  2. �䤤�� b_lines+shift �C�O���ܪ����e�A�� rscroll �F��
                  3. �b�e���� outs_line() ���a��L�X -shift �C
                  4. �̫�A�첾��~ rscroll ���C��
                */

                /* ���첾��W�@�� block ������ */
                i = BMIN((lino - b_lines) >> 5, MAXBLOCK - 1);
                foff = fimage + block[i];
                i = i * 32;

                /* �A�q�W�@�� block �����ݦ첾�� lino-b_lines+1 �C */
                for (i = lino - b_lines - i; i > 0; i--)
                    more_line(buf);

                for (i = shift; i < 0; i++)
                {
                    rscroll();
                    move(0, 0);
                    clrtoeol();
                }

                i = shift;
            }
            else                        /* Home */
            {
                /* itoc.041226.����: �ؼЬO�q�X 1 ~ b_lines �C�����e�G
                   �@�k�N�O�������k�s�A�q�Y�A�L b_lines �C�Y�i */

                clear();

                foff = fimage;
                lino = 0;
                shift = b_lines;
            }
        }
        else                            /* ��ø footer �� re-key */
        {
            move(b_lines, 0);
            clrtoeol();
            goto re_key;
        }
    }   /* while �j�骺���� */

    /* --------------------------------------------------- */
    /* �ɮפw�g�q�� (cmd = 0) �� �ϥΪ̤��_ (cmd != 0)     */
    /* --------------------------------------------------- */

    free(fimage);

#endif // M3_USE_PMORE

    if (!cmd)   /* �ɮץ��`�q���A�n�B�z footer */
    {
        if (footer)             /* �� footer */
        {
            if (footer != (char *) -1)
                outf(footer);
            else
                outs(str_ransi);
        }
        else            /* �S�� footer �n vmsg() */
        {
            /* lkchu.981201: ���M�@���H�K���|��� */
            move(b_lines, 0);
            clrtoeol();

            if (vmsg(NULL) == 'C')      /* Thor.990204: �S�O�`�N�Y�^�� 'C' ��ܼȦs�� */
            {
                FILE *fp;

                if ((fp = tbf_open(-1)))
                {
                    f_suck(fp, fpath);
                    fclose(fp);
                }
            }
        }
    }
    else                /* �ϥΪ̤��_�A�������} */
    {
        outs(str_ransi);
    }

    hunt[0] = '\0';

    /* Thor.990204: ��key�i�^�Ǧ�more�~ */
    return cmd;
}
