/*-------------------------------------------------------*/
/* window.c     ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* target : popup window menu                            */
/* create : 03/02/12                                     */
/* update : 03/07/23                                     */
/* author : verit.bbs@bbs.yzu.edu.tw                     */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/

/* 90929.cache: �ĤG�عϧο�� */

#include "bbs.h"

#ifdef M3_USE_PFTERM
/* ----------------------------------------------------- */
/* M3_USE_PFTERM                                         */
/* ----------------------------------------------------- */

static void
draw_line(
    int y, int x,
    const char *msg)
{
    /* hrs.090928: �� terminal �h�B�z */
    move(y, x);
    outstr(msg);
    return;
}

#else

static int x_roll;

/* ----------------------------------------------------- */
/* �e��ø�s                                              */
/* ----------------------------------------------------- */


static void
draw_line(              /* �b (y, x) ����m��J msg�A���k���n�L�X��Ӫ��m���r */
    int y, int x,
    const char *msg)
{
    char *str;
    const char *ptr;
    char data[ANSILINESIZE];
    char color[4];
    int ch, i;
    int len;
    int ansi;           /* 1: �b ANSI �� */
    int in_dbcs = 0;    /* 1: �b����r�� */
    int fg, bg, hl;     /* �e��/�I��/���m */
    screenline slt;

    hl = 0;
    fg = 37;
    bg = 40;

    i = y + x_roll;
    if (i > b_lines)
        i -= b_lines + 1;

    vs_save_line(&slt, i);
    str_sncpy(data, (char *) slt.data, sizeof(data), slt.len);
    str = data;

    move(y, 0);
    clrtoeol();

    /* �L�X (y, 0) �� (y, x - 1) */
    ansi = 0;
    len = 0;            /* �w�L�X�X�Ӧr (���t����X) */
    while ((ch = (unsigned char) *str++))
    {
        if (ch == KEY_ESC)
        {
            ansi = 1;
            i = 0;
        }
        else if (ansi)
        {
            if (ch == '[')
            {
            }
            else if (ch >= '0' && ch <= '9')
            {
                color[i] = ch;
                if (++i >= 4)
                    i = 0;
            }
            else
            {
                color[i] = 0;

                i = atoi(color);
                if (i == 0)
                {
                    hl = 0;
                    fg = 37;
                    bg = 40;
                }
                else if (i == 1)
                    hl = 1;
                else if (i >= 30 && i <= 37)
                    fg = i;
                else if (i >= 40 && i <= 47)
                    bg = i;

                i = 0;

                if (ch != ';')
                    ansi = 0;
            }
        }
        else
        {
            if (++len >= x)
            {
                /* �̫�@�r�Y�O����r�����X�A�N���L */
                if (!in_dbcs && IS_DBCS_HI(ch))
                {
                    outc(' ');
                    in_dbcs ^= 1;
                }
                else
                {
                    outc(ch);
                    in_dbcs = 0;
                }
                outs(str_ransi);
                break;
            }

            if (in_dbcs || IS_DBCS_HI(ch))
                in_dbcs ^= 1;
        }

        outc(ch);
    }
    while (len++ < x)
        outc(' ');

    /* �L�X (y, x) �� (y, x + strip_ansi_len(msg) - 1) */
    ptr = msg;
    ansi = 0;
    len = 0;            /* msg ������(���t����X) */
    while ((ch = (unsigned char) *ptr++))
    {
        if (ch == KEY_ESC)
        {
            ansi = 1;
        }
        else if (ansi)
        {
            if ((ch < '0' || ch > '9') && ch != ';' && ch != '[')
                ansi = 0;
        }
        else
        {
            len++;
        }
        outc(ch);
    }

    /* ���� str �����@��q�A�è��X�̫᪺�C�� */
    ansi = 0;
    while ((ch = (unsigned char) *str++))
    {
        if (ch == KEY_ESC)
        {
            ansi = 1;
            i = 0;
        }
        else if (ansi)
        {
            if (ch == '[')
                continue;
            if (ch >= '0' && ch <= '9')
            {
                color[i] = ch;
                if (++i >= 4)
                    i = 0;
            }
            else
            {
                color[i] = 0;

                i = atoi(color);
                if (i == 0)
                {
                    hl = 0;
                    fg = 37;
                    bg = 40;
                }
                else if (i == 1)
                    hl = 1;
                else if (i >= 30 && i <= 37)
                    fg = i;
                else if (i >= 40 && i <= 47)
                    bg = i;

                i = 0;

                if (ch != ';')
                    ansi = 0;
            }
        }
        else
        {
            if (--len < 0)      /* ���L strip_ansi_len(msg) ������ */
                break;

            if (in_dbcs || IS_DBCS_HI(ch))
                in_dbcs ^= 1;
        }
    }

    /* �L�X (y, x + strip_ansi_len(msg)) �o�Ӧr�Ϋ᭱������X */
    prints("\x1b[%d;%d;%dm", hl, fg, bg);
    /* ���r�Y�O����r�����X�A�N���L */
    outc(in_dbcs ? ' ' : ch);

    /* �L�X (y, x + strip_ansi_len(msg) + 1) �� ��� */
    outs(str);
    outs(str_ransi);
}
#endif //M3_USE_PFTERM

/* ----------------------------------------------------- */
/* �ﶵø�s                                              */
/* ----------------------------------------------------- */


static void
draw_item(
    int y, int x,
    const char *desc,
    char hotkey,
    int mode)           /* 0:�M������  1:�e�W���� */
{
    char buf[128];

    sprintf(buf, " �x%s%c %c%c%c%-25s  \x1b[m�x ",
        mode ? COLOR4 : "\x1b[30;47m", mode ? '>' : ' ',
        (hotkey == *desc) ? '[' : '(', *desc,
        (hotkey == *desc) ? ']' : ')', desc + 1);

    draw_line(y, x, buf);
}


static int      /* �^���`�@���X�ӿﶵ */
draw_menu(
    int y, int x,
    const char *title,
    const char *const desc[],
    char hotkey,
    int *cur)   /* �^�ǹw�]�ȩҦb��m */
{
    int i, meet;
    char buf[128];

    draw_line(y++, x, " �~�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�� ");

    sprintf(buf, " �x" COLOR4 "  %-28s  \x1b[m�x ", title);
    draw_line(y++, x, buf);

    draw_line(y++, x, " �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t ");

    for (i = 1; desc[i]; i++)
    {
        meet = (desc[i][0] == hotkey);
        draw_item(y++, x, desc[i], hotkey, meet);
        if (meet)
            *cur = i;
    }

    draw_line(y, x, " ���w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�� ");

    /* �קK�b�������k����ΤU�A������|�����G�h��檺���D */
    move(b_lines, 0);

    return i - 1;
}


/* ----------------------------------------------------- */
/* ��ﶵ                                                */
/* ----------------------------------------------------- */


static int                      /* -1:�䤣�� >=0:�ĴX�ӿﶵ */
find_cur(               /* �� ch �o�ӫ���O�ĴX�ӿﶵ */
    int ch, int max,
    const char *const desc[])
{
    int i, cc;

    if (ch >= 'A' && ch <= 'Z')
        ch |= 0x20;             /* ���p�g */

    for (i = 1; i <= max; i++)
    {
        cc = desc[i][0];
        if (cc >= 'A' && cc <= 'Z')
            cc |= 0x20; /* ���p�g */

        if (ch == cc)
            return i;
    }

    return -1;
}


/*------------------------------------------------------ */
/* �߰ݿﶵ�A�i�ΨӨ��N vans()                           */
/*------------------------------------------------------ */
/* y, x  �O�ۥX�������W���� (y, x) ��m                  */
/* title �O���������D                                    */
/* desc  �O�ﶵ���ԭz�G                                  */
/*       �Ĥ@�Ӧr�ꥲ������� char                       */
/*         �Ĥ@�Ӧr���N��@�}�l��а�����m              */
/*         �ĤG�Ӧr���N����U KEY_LEFT ���w�]�^�ǭ�      */
/*       �������r��O�C�ӿﶵ���ԭz (���r��������)       */
/*       �̫�@�Ӧr�ꥲ���� NULL                         */
/*------------------------------------------------------ */

/* IID.20200204: Use screen size referencing coordinate */
int             /* �Ǧ^�p�g�r���μƦr */
popupmenu_ans2(const char *const desc[], const char *title, int y_ref, int x_ref)
{
    int y, x;
    int cur, old_cur, max;
    int ch = KEY_NONE;
    char hotkey;

    screen_backup_t old_screen;
    screen_backup_t old_screen_dark;

#ifndef M3_USE_PFTERM
    x_roll =
#endif
    scr_dump(&old_screen);

    grayout(0, b_lines, GRAYOUT_DARK);
    scr_dump(&old_screen_dark);

    hotkey = desc[0][0];

popupmenu_ans2_redraw:
    y = gety_ref(y_ref);
    x = getx_ref(x_ref);

    /* �e�X��ӿ�� */
    max = draw_menu(y, x, title, desc, hotkey, (ch != I_RESIZETERM) ? &cur : &old_cur);
    y += 2;

    /* �@�i�J�A��а��b�w�]�� */
    if (ch != I_RESIZETERM)
        old_cur = cur;

    while (1)
    {
        if (old_cur != cur)             /* ����ܰʦ�m�~�ݭn��ø */
        {
            draw_item(y + old_cur, x, desc[old_cur], hotkey, 0);
            draw_item(y + cur, x, desc[cur], hotkey, 1);
            old_cur = cur;
            /* �קK�b�������k����ΤU�A������|�����G�h��檺���D */
            move(b_lines, 0);
        }

        ch = vkey();
        if (ch == I_RESIZETERM)
        {
            /* Screen size changed and redraw is needed */
            /* clear */
            scr_restore_keep(&old_screen_dark);
            goto popupmenu_ans2_redraw;
        }

        switch (ch)
        {
        case KEY_LEFT:
        case KEY_ESC:
        case Meta(KEY_ESC):
        case KEY_RIGHT:
        case '\n':
            scr_free(&old_screen_dark);
            scr_restore_free(&old_screen);
            ch = (ch == KEY_RIGHT || ch == '\n') ? desc[cur][0] : desc[0][1];
            if (ch >= 'A' && ch <= 'Z')
                ch |= 0x20;             /* �^�Ǥp�g */
            return ch;

        case KEY_UP:
            cur = (cur == 1) ? max : cur - 1;
            break;

        case KEY_DOWN:
            cur = (cur == max) ? 1 : cur + 1;
            break;

        case KEY_HOME:
            cur = 1;
            break;

        case KEY_END:
            cur = max;
            break;

        default:                /* �h��ҫ���O���@�ӿﶵ */
            if ((ch = find_cur(ch, max, desc)) > 0)
                cur = ch;
            break;
        }
    }
}

/*------------------------------------------------------ */
/* �ۥX�������T���A�i�ΨӨ��N vmsg()                     */
/*------------------------------------------------------ */

// IID.20190909: `pmsg2()` without blocking and without screen restoring.
void
pmsg2_body(const char *msg)
/* ���i�� NULL */
{
    int len, y, x, i;
    char buf[80];

    if (!msg)
    {
        vmsg_body(NULL);
        return;
    }

    grayout(0, b_lines, GRAYOUT_DARK);

    len = BMAX(strlen(msg), (size_t)16); /* �� msg title �䤤�����̬� len */
    if (len % 2)                /* �ܦ����� */
        len++;
    y = (b_lines - 4) >> 1;     /* �m�� */
    x = (b_cols - 8 - len) >> 1;

    strcpy(buf, "�~");
    for (i = -4; i < len; i += 2)
        strcat(buf, "�w");
    strcat(buf, "��");
    draw_line(y++, x, buf);

    sprintf(buf, "�x" COLOR4 "  %-*s  \x1b[m�x", len, "�Ы����N���~��..");
    draw_line(y++, x, buf);

    strcpy(buf, "�u");
    for (i = -4; i < len; i += 2)
        strcat(buf, "�w");
    strcat(buf, "�t");
    draw_line(y++, x, buf);

    sprintf(buf, "�x\x1b[30;47m  %-*s  \x1b[m�x", len, msg);
    draw_line(y++, x, buf);

    strcpy(buf, "��");
    for (i = -4; i < len; i += 2)
        strcat(buf, "�w");
    strcat(buf, "��");
    draw_line(y++, x, buf);

    move(b_lines, 0);
}

int
pmsg2(const char *msg)
/* ���i�� NULL */
{
    int x;

    if (!msg)
        return vmsg(NULL);

    screen_backup_t old_screen;

#ifndef M3_USE_PFTERM
    x_roll =
#endif
    scr_dump(&old_screen);

    for (;;)
    {
        pmsg2_body(msg);
        if ((x = vkey()) != I_RESIZETERM)
            break;
        scr_restore_keep(&old_screen);
    }

    scr_restore_free(&old_screen);
    return x;
}
