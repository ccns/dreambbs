/************************************************
*  pnote.c                                      *
*  留話機                                       *
*                                1998.4.14      *
*                                    by herb    *
*************************************************/

#include "bbs.h"
#define MAX_PNOTE        (10)            /* 答錄機保留通數 */
#define MAXHINTLINES     (10)            /* 答錄機主人留言長度 */

static const char *const fn_note_tmp      = FN_PNOTE_TMP;
static const char *const fn_note_dat      = FN_PNOTE_DAT;
static const char *const fn_note_dat_save = FN_PNOTE_DAT_SAVE;
static const char *const fn_pnote_ans      = FN_PNOTE_ANS;
static const char *const fn_pnote_ans_save = FN_PNOTE_ANS_SAVE;

const char *fn_pnote_hint = FN_PNOTE_HINT;    /* 答錄機主人留言 */

char Bdate[128];
#include "pipfun.c"

#define getdata(x1, x2, x3, x4, x5, x6, x7)  vget(x1, x2, x3, x4, x5, DOECHO)
#define setuserfile(x1, x2)  usr_fpath(x1, cuser.userid, x2)

static void
m_biffn(
int userno)
{
    UTMP *utmp, *uceil;

    utmp = ushm->uslot;
    uceil = (UTMP *) ((char *) utmp + ushm->offset);
    do
    {
        if (utmp->userno == userno)
        {
            utmp->ufo |= UFO_BIFFN;

#ifdef  BELL_ONCE_ONLY
            return;
#endif
        }
    }
    while (++utmp <= uceil);
}

static void
rebuild_pnote_ansi(int newflag)
{
    char fname[MAXPATHLEN];
    char fpath[MAXPATHLEN];
    char buf[256], buf2[80];
    int total = 0, len, i;
    int fd;
    struct stat st;
    notedata myitem;
    FILE *fp;

    if (newflag)
    {
        setuserfile(fname, fn_pnote_ans);
        setuserfile(fpath, fn_note_dat);
    }
    else
    {
        setuserfile(fname, fn_pnote_ans_save);
        setuserfile(fpath, fn_note_dat_save);
    }

    if ((fp = fopen(fname, "w")) == NULL)
    {
        return;
    }

    if ((fd = open(fpath, O_RDONLY)) == -1)
    {
        fclose(fp);
        unlink(fname);
        return;
    }
    else if (fstat(fd, &st) != -1)
    {
        total = BMIN(st.st_size / sizeof(notedata), (off_t)MAX_PNOTE);
    }
    fputs("\t\t\t\x1b[1;32m ★ \x1b[37m答 錄 機 中 的 留 言\x1b[32m★ \n\n", fp);

    while (total)
    {
        if (total--)
            read(fd, (char *) &myitem, sizeof(myitem));
        sprintf(buf, "\x1b[1;33m□═ \x1b[32m%s\x1b[37m(%s)",
                myitem.userid, myitem.username);
        len = strlen(buf);
        strcat(buf, & " \x1b[33m"[len & 1]);

        for (i = len >> 1; i < 36; i++)
            strcat(buf, "═");
        sprintf(buf2, "═\x1b[32m %.14s \x1b[33m═□\x1b[m\n",
                Cdate(&(myitem.date)));
        strcat(buf, buf2);
        fputs(buf, fp);

        sprintf(buf, "%s\n%s\n%s\n", myitem.buf[0], myitem.buf[1], myitem.buf[2]);
        fputs(buf, fp);

    }
    fclose(fp);
    close(fd);
}

static void
do_pnote(const char *userid)
{
    int total = 0, i, collect GCC_UNUSED, len;
    struct stat st;
    char buf[256], buf2[80];
    char fname[MAXPATHLEN], fname2[MAXPATHLEN];
    int fd, fx;
    FILE *fp;
    notedata myitem;

    clrtobot();
    move(13, 0);
    outs("\x1b[1;33m口畢～～～！\x1b[m");
    memset(&myitem, 0, sizeof(notedata));
    do
    {
        /*    myitem.buf[0][0] = myitem.buf[1][0] = myitem.buf[2][0] = '\0';*/
        move(14, 0);
        clrtobot();
        outs("\n請留話 (至多三句)，按[Enter]結束");
        for (i = 0; (i < 3) &&
            getdata(16 + i, 0, ":", myitem.buf[i], 77, DOECHO, 0); i++);

        getdata(B_LINES_REF, 0, "(S)儲存 (E)重新來過 (Q)取消？[S] ", buf, 3, LCECHO, 0);
        if ((buf[0] == 'q' || i == 0) && *buf != 'e')
        {
            return;
        }
        myitem.mode = 0;
    }
    while (buf[0] == 'e');

    utmp_mode(M_XMODE);
    strcpy(myitem.userid, cuser.userid);
    strncpy(myitem.username, cuser.username, 18);
    myitem.username[18] = '\0';
    time(&(myitem.date));

    /* begin load file */

    usr_fpath(fname, userid, fn_pnote_ans);
    if ((fp = fopen(fname, "w")) == NULL)
    {
        return;
    }

    usr_fpath(fname, userid, fn_note_tmp);
    if ((fx = open(fname, O_WRONLY | O_CREAT, 0644)) < 0)
    {
        fclose(fp);
        return;
    }

    usr_fpath(fname2, userid, fn_note_dat);
    if ((fd = open(fname2, O_RDONLY)) == -1)
    {
        total = 1;
    }
    else if (fstat(fd, &st) != -1)
    {
        total = BMIN(st.st_size / sizeof(notedata) + 1, (off_t)MAX_PNOTE);
    }

    fputs("\t\t\t\x1b[1;32m ★ \x1b[37m您 的 答 錄 機 !!! \x1b[32m★ \n\n", fp);
    collect = 1;
    while (total)
    {
        sprintf(buf, "\x1b[1;33m□═ \x1b[32m%s\x1b[37m(%s)",
                myitem.userid, myitem.username);
        len = strlen(buf);
        strcat(buf, & " \x1b[33m"[len & 1]);

        for (i = len >> 1; i < 36; i++)
            strcat(buf, "═");
        sprintf(buf2, "═\x1b[32m %.14s \x1b[33m═□\x1b[m\n",
                Cdate(&(myitem.date)));
        strcat(buf, buf2);
        fputs(buf, fp);

        sprintf(buf, "%s\n%s\n%s\n", myitem.buf[0], myitem.buf[1], myitem.buf[2]);
        fputs(buf, fp);

        write(fx, &myitem, sizeof(myitem));

        if (--total)
            read(fd, (char *) &myitem, sizeof(myitem));
    }
    fclose(fp);
    if (fd >= 0)
        close(fd);
    close(fx);
    f_mv(fname, fname2);
    m_biffn(acct_userno(userid));
}

static void
show_pnote(notedata *pitem)
{
    clrchyiuan(2, 6);
    move(2, 0);
    prints_centered("\x1b[1;36m┌─── \x1b[37m%s(%s)在 \x1b[33m%s\x1b[37m 留的話 \x1b[m", pitem->userid, pitem->username,
            Cdate(&(pitem->date)));
    prints("\n\x1b[1;37m%*s  %s\n%*s  %s\n%*s  %s\n\x1b[0m",
           d_cols>>1, "", pitem->buf[0], d_cols>>1, "", pitem->buf[1], d_cols>>1, "", pitem->buf[2]);
    outs_centered("                 \x1b[1;36m──────────────────────────────┘\x1b[m\n");
    pitem->mode = 1;
}

#if 0
static void
del_pnote(notedata *pitem, int newflag)
{
    char fpath[MAXPATHLEN];
    char fold[MAXPATHLEN];
    FILE *fp1, *fp2;
    notedata item;

    if (newflag)
        setuserfile(fpath, fn_note_dat);
    else
        setuserfile(fpath, fn_note_dat_save);

    sprintf(fold, "%s.tmp", fpath);
    f_mv(fpath, fold);
    if ((fp1 = fopen(fold, "r")) != NULL)
    {
        if ((fp2 = fopen(fpath, "w")) != NULL)
        {
            while (fread(&item, sizeof(item), 1, fp1) != 0)
            {
                if (memcmp(pitem, &item, sizeof(item)))
                {
                    fwrite(&item, sizeof(item), 1, fp2);
                }
            }
            fclose(fp2);
        }
        fclose(fp1);
    }
    unlink(fold);
}
#endif  /* #if 0 */

/*                                                              *
 *  show_pnote_hint()改自showplan(), 可以show出主人自製的答錄機 *
 *  留言, 若無留言則會show預設留言                              *
 *                                          - add by wisely -   */

static void
show_pnote_hint(
const char *uid)
{
    FILE *hintfile;
    int i;
    char genbuf[256];

    usr_fpath(genbuf, uid, fn_pnote_hint);
    if ((hintfile = fopen(genbuf, "r")))
    {
        outs_centered("\x1b[1;34m●────────────────────────────────●\x1b[m\n");
        i = 0;
        while (i++ < MAXHINTLINES && fgets(genbuf, 256, hintfile))
        {
            outs_centered(genbuf);
        }
        outs_centered("\x1b[1;34m●────────────────────────────────●\x1b[m\n");
        fclose(hintfile);
    }
    else
        prints("您好，這是 %s 的電話答錄機，", uid);
    outs("\n請在聽到「嗶」聲後，開始留言，謝謝。");
}

/*                                                              *
 *  p_edithint()改自u_editplan(), 可以修改主人留言              *
 *                                      - add by wisely         */

static int
p_edithint(void)
{
    char genbuf[200];

    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("你還有檔案還沒編完哦！");
        return -1;
    }


    sprintf(genbuf, "主人留言(最多%d行) (D)刪除 (E)錄製 [Q]取消？[Q]", MAXHINTLINES);
    getdata(B_LINES_REF, 0, genbuf, genbuf, 3, LCECHO, 0);

    if (genbuf[0] == 'e')
    {
        int aborted;

        utmp_mode(M_XMODE);
        setuserfile(genbuf, fn_pnote_hint);
        aborted = vedit(genbuf, false);
        if (!aborted)
            outs("留言錄製完畢");
        vmsg(NULL);
        return 0;
    }
    else if (genbuf[0] == 'd')
    {
        setuserfile(genbuf, fn_pnote_hint);
        unlink(genbuf);
        vmsg("留言刪除完畢");
    }
    return 0;
}


static int
get_pnote(notedata *pitem, int newflag)
{
    FILE *fp;
    int  total = 0;
    notedata myitem;
    char fpath[MAXPATHLEN];

    if (newflag)
        setuserfile(fpath, fn_note_dat);
    else
        setuserfile(fpath, fn_note_dat_save);

    if ((fp = fopen(fpath, "r")) != NULL)
    {
        while (fread(&myitem, sizeof(myitem), 1, fp) == 1)
        {
            memcpy(&pitem[total++], &myitem, sizeof(notedata));
        }
        fclose(fp);
        pitem[total].userid[0] = '\0';
        return total;
    }
    return 0;
}


static void
Pnote(int newflag)
{
    int offset, num, num1, i;
    char ans[4], prompt[STRLEN];
    notedata item_array[MAX_PNOTE + 1];
    char fpath[MAXPATHLEN];
    FILE *fp;

    if ((num = get_pnote(item_array, newflag)) == 0)
        return;

    utmp_mode(M_XMODE);
    rebuild_pnote_ansi(newflag);
    offset = 1;
    while (offset <= num)
    {
        move(1, 0);
        clrtobot();
        prints("\x1b[1;36m答錄機裡的\x1b[37m%2d/%2d\x1b[36m通%s留言\x1b[0m\n", offset, num, newflag ? "新的" : "");

        show_pnote(&item_array[offset - 1]);
        sprintf(prompt, "(N/P)往前/後 (A)全部 (R)回電 %s(X)刪除全部 (E)離開:", newflag ? "(S)保留 " : "(D)刪除 ");
        getdata(B_LINES_REF, 0, prompt, ans, 2, DOECHO, 0);

        if (ans[0] == 'r' || ans[0] == 'R')
        {
            do_pnote(item_array[offset - 1].userid);
            offset++;
        }
        else if (ans[0] == 'p' || ans[0] == 'P')
        {
            offset = BMAX(offset-1, 1);
        }
#if 0
        else if (ans[0] == 'q' || ans[0] == 'Q')
        {
            my_query(item_array[offset - 1].userid, 0);
            offset++;
        }
#endif
        else if (ans[0] == 'a' || ans[0] == 'A')
        {
            for (i = 0; i < num; i++)
                item_array[i].mode = 0;
            if (newflag)
                setuserfile(fpath, fn_pnote_ans);
            else
                setuserfile(fpath, fn_pnote_ans_save);
            more(fpath, NULL);
            break;
        }
        else if ((ans[0] == 'd' || ans[0] == 'D') && !newflag)
        {
            for (i = offset - 1; i < num; i++)
                memcpy(&item_array[i], &item_array[i+1], sizeof(notedata));

            num--;
            if (num == 0)
                break;
            offset = BMIN(offset, num);
        }
        else if ((ans[0] == 's' || ans[0] == 'S') && newflag)
        {
            setuserfile(fpath, fn_note_dat_save);
            if ((num1 = get_pnote(item_array, 0)) >= MAX_PNOTE)
            {
                move(b_lines - 1, 0);
                vmsg("答錄機已經錄到底囉，沒辦法保存了，記得快整理整理喔...");
                break;
            }
            else if ((num1 = get_pnote(item_array, 0)) >= MAX_PNOTE - 3)
            {
                move(b_lines - 1, 0);
                vmsg("答錄機快滿了，記得清理清理喔...");
            }
            // shakalaca patch [原本顯示的內容都變成第一篇]
            get_pnote(item_array, 1);
            if ((fp = fopen(fpath, "a")) != NULL)
            {
                if (fwrite(&item_array[offset - 1], sizeof(notedata), 1, fp) != 1)
                    break;

                fclose(fp);
            }
            for (i = offset - 1; i < num; i++)
                memcpy(&item_array[i], &item_array[i+1], sizeof(notedata));

            num--;
            if (num == 0)
                break;
            offset = BMIN(offset, num);

        }
#if 0
        else if (ans[0] == 'm'  || ans[0] == 'M')
        {
            fileheader mymail;
            char title[128], buf[80];

            if (newflag)
                setuserfile(fpath, fn_pnote_ans);
            else
                setuserfile(fpath, fn_pnote_ans_save);

            sethomepath(buf, cuser.userid);
            stampfile(buf, &mymail);

            mymail.savemode = 'H';        /* hold-mail flag */
            mymail.filemode = FILE_READ;
            strcpy(mymail.owner, "[備.忘.錄]");
            strcpy(mymail.title, "留言\x1b[37;41m記錄\x1b[m");
            sethomedir(title, cuser.userid);
            rec_add(title, &mymail, sizeof(mymail));
            f_mv(fpath, buf);
        }
#endif
        else if (ans[0] == 'x' || ans[0] == 'X')
        {
            item_array[0].userid[0] = '\0';
            break;
        }
        else if (ans[0] == 'e' || ans[0] == 'E')
        {
            break;
        }
        else
        {
            offset++;
        }
        offset = BMIN(offset, num);
    }

    if (newflag)
        setuserfile(fpath, fn_note_dat);
    else
        setuserfile(fpath, fn_note_dat_save);

    offset = 0;
    if ((fp = fopen(fpath, "w")) != NULL)
    {
        while (item_array[offset].userid[0] != '\0')
        {
            if (newflag && item_array[offset].mode != 0)
            {
                offset++;
                continue;
            }

            if (fwrite(&item_array[offset], sizeof(notedata), 1, fp) != 1)
                break;

            offset++;
        }
        fclose(fp);
    }
    rebuild_pnote_ansi(newflag);
    return;
}

/*                                                      *
 *  p_call(), 一個簡單的留言介面, 加在menu中就可以用了  *
 *  欲加入其它的傳呼功能可由此加入.                     *
 *                          - add by wisely 5/5/98 -    */

static int
p_call(void)
{
    char genbuf[200];

    vs_bar("留言給.....");
    usercomplete("你想留言給誰：", genbuf);

    if (genbuf[0])
    {
        clear();
        move(1, 0);
        show_pnote_hint(genbuf);
        do_pnote(genbuf);
    }
    return 0;
}

/*
 * p_read() 是設計用來可以擺在 menu 中的. 只要把他加在想加的menu裡
 * 就可以使用囉.
 */
static int
p_read(void)
{
    char ans[4];
    char prompt[STRLEN];

    sprintf(prompt, "(N)新的留言/(S)被保存的留言 [%c]", check_personal_note(1, NULL) ? 'N' : 'S');
    getdata(B_LINES_REF, 0, prompt, ans, 2, DOECHO, 0);
    if (ans[0] == 'n')
        Pnote(1);
    else if (ans[0] == 's')
        Pnote(0);
    else if (check_personal_note(1, NULL))
        Pnote(1);
    else
        Pnote(0);

    return 0;
}

int
main_note(void)
{
    DL_HOLD;
    char ans;
    ans = vans("個人答錄機：R)閱\讀留言 C)錄製留言 E)更改問候語 Q)離開 [Q]：");
    switch (ans)
    {
case 'r': case 'R':
        cutmp->ufo &= ~UFO_BIFFN;
        p_read();
        break;
case 'c': case 'C':
        p_call();
        break;
case 'e': case 'E':
        p_edithint();
        break;
    }
    return DL_RELEASE(0);
}
