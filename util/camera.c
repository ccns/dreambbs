/*-------------------------------------------------------*/
/* util/camera.c        ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* target : 建立 [動態看板] cache                        */
/* create : 95/03/29                                     */
/* update : 97/03/29                                     */
/*-------------------------------------------------------*/


#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <strings.h>


#include "bbs.h"
#include "dao.h"


static const char *list[] = {
    "welcome",
    "bye",
    "apply",
    "tryout",
    "post",
    "gem.hlp",
    "board.hlp",
    "class.hlp",
    "friend.hlp",
    "mbox.hlp",
    "ulist.hlp",
    "vote.hlp",
    "more.hlp",
    "edit.hlp",
    "bmw.hlp",
    "banmail.hlp",
    "income",
    "admin.hlp",
    "song.hlp",
    "mime.hlp",
    "contact.hlp",
    "memorandum.hlp",
    "aloha.hlp",
    "signup.hlp",
//    NULL
    "myfav.hlp",    /* r2.20170727: test add back my favorite*/
    NULL
};


#define MAX_LINE        MOVIE_LINES


static FCACHE image;
static int number;
static int tail;


    static void
mirror(
    char *fpath)
{
    int fd, size;
    char *ptr;

    if (number >= MOVIE_MAX - 1)
        return;

    fd = open(fpath, O_RDONLY);
    if (fd >= 0)
    {
        ptr = image.film + tail;
        size = read(fd, ptr, FILM_SIZ);
        close(fd);

        if (size <= 0)
            return;

        ptr[size] = '\0';
        size = str_rle(ptr);

        if (size > 0 && size < FILM_SIZ)
        {
            ptr[size++] = '\0';
            image.shot[++number] = (tail += size);
        }
    }
}


    static int
play(
    char *data)
{
    int line, ch;
    char *head;

    if (number >= MOVIE_MAX -1)
        return 1;

    /* str_rle(data); */
    /* Thor.980804: 等算完最後再來壓縮, 不然壓縮碼也被視作換行或行數不夠了 */

    head = data;
    line = 0;
    while ((ch = *data))                /* at most 10 lines */
    {
        data++;
        if (ch == '\n')
        {
            if (++line >= MAX_LINE)
                break;
        }
    }

    *data++ = 27;
    *data++ = '[';
    *data++ = 'm';

    while (line < MAX_LINE)     /* at lease 10 lines */
    {
        *data++ = '\n';
        line++;
    }

    *data = '\0';

    /* *data++ = '\0';*/                /* mark for end of movie */

    /* ch = data - head; */             /* length */

    ch = str_rle(head) + 1; /* Thor.980804: +1 將結尾的0也算入 */

    line = tail + ch;
    if (line >= MOVIE_SIZE)
        return 1;                       /* overflow */

    data = image.film + tail;
    memcpy(data, head, ch);
    image.shot[++number] = tail = line;
    return 0;
}

    int
main(
    int argc,      /* Thor.980804: 為了讀取參數 */
    char *argv[])
{
    int i, fd, size;
#ifdef      HAVE_SONG_TO_CAMERA
    int j, k;
#endif
    char *ptr, *fname, fpath[80], buf[FILM_SIZ + 1];
    const char *str;
#ifdef  HAVE_RAND_INCOME
    char fincome[128];
    int pos;
    time_t now;
    struct stat st;
    struct tm *xtime, ptime;
#endif
    FCACHE *fshm;
    FILE *fp;
    HDR hdr;


    srand(time(0));

    /* --------------------------------------------------- */
    /* mirror pictures                                     */
    /* --------------------------------------------------- */


    setgid(BBSGID);
    setuid(BBSUID);
    chdir(BBSHOME);

    strcpy(fpath, "gem/@/@");
    fname = fpath + 7;

    for (i = 0; (str = list[i]); i++)
    {
        strcpy(fname, str);
#ifdef  HAVE_RAND_INCOME
        if (!strcmp(str, "income"))
        {
            now = time(NULL);
            xtime = localtime(&now);
            ptime = *xtime;
            sprintf(fincome, "gem/brd/%s/@/@income", BRD_CAMERA);
            fd = open(fincome, O_RDONLY);
            if ((fd>=0) && !fstat(fd, &st) && (size = st.st_size) > 0)
            {
                size /= sizeof(HDR);
                pos = ((rand() / size) + ptime.tm_mon + ptime.tm_mday + ptime.tm_year + rand()) % size;
                if (pos < 0)
                    pos += size;
                lseek(fd, (off_t) (sizeof(HDR) * pos), SEEK_SET);
                read(fd, &hdr, sizeof(HDR));
                ptr = strchr(fincome, '@');
                *ptr = hdr.xname[7];
                strcpy(ptr + 2, hdr.xname);
                mirror(fincome);
            }
            else
                mirror(fpath);
            if (fd>=0)
                close(fd);
        }
        else
#endif
            mirror(fpath);
    }

    /* --------------------------------------------------- */
    /* visit all films                                     */
    /* --------------------------------------------------- */
    i = FILM_MOVIE;

    sprintf(fpath, "gem/brd/%s/@/@note", BRD_CAMERA);

#ifdef  HAVE_SONG_TO_CAMERA
    for (j=0; j<=1; j++)
    {
#endif
        if ((fp = fopen(fpath, "r")))
        {
#ifdef  HAVE_SONG_TO_CAMERA
            if (j==1) sprintf(fpath, "brd/%s/@/", BRD_ORDERSONGS);
#endif
            ptr = strchr(fpath, '@');
            while (fread(&hdr, sizeof hdr, 1, fp) == 1)
            {
                /* Thor.981110: 限制級則不放入movie中 */
                if (hdr.xmode & (GEM_RESTRICT|GEM_LOCK))
                    continue;

                *ptr = hdr.xname[7];
                strcpy(ptr + 2, hdr.xname);
                if ((fd = open(fpath, O_RDONLY)) >= 0)
                {
                    /* 讀入檔案 */

                    size = read(fd, buf, FILM_SIZ);
                    close(fd);

                    if (size >= FILM_SIZ || size <= 0)
                        continue;

                    buf[size] = '\0';
                    ptr = buf;

#ifdef  HAVE_SONG_TO_CAMERA
                    if (j == 1 /*&& !strncmp(buf, str_author1, strlen(str_author1))*/)
                    {
                        for (k=0; k<=3 && ptr; k++)
                        {
                            ptr = strchr(ptr, '\n');
                            if (ptr)
                                ptr++;
                        }
                        if (!ptr)
                            continue;
                    }
#endif

                    if (play(ptr))      /* overflow */
                        break;
                    if (++i >= MOVIE_MAX) /* Thor.980804: 為什麼不乾脆用 number算了?:P */
                        break;
                }
            }
            fclose(fp);
        }
        else
        {
            if ((fd = open(FN_ERROR_CAMERA, O_RDONLY)) >= 0)
            {

                size = read(fd, buf, FILM_SIZ);
                close(fd);

                if (size > 0)
                {
                    buf[size] = '\0';
                    play(buf);
                }
            }
            else
            { /* 針對當沒有 @error-camera 時的處理 */
                strcpy(buf, "動態看版錯誤\n請聯絡系統管理員\n");
                play(buf);
            }
        }
#ifdef      HAVE_SONG_TO_CAMERA
        sprintf(fpath, "brd/%s/.DIR", BRD_ORDERSONGS);
    }
#endif
    i = number; /* 總共有幾片 ? */

    /* --------------------------------------------------- */
    /* resolve shared memory                               */
    /* --------------------------------------------------- */

    fshm = (FCACHE *) shm_new(FILMSHM_KEY, sizeof(FCACHE));
    memcpy(fshm, &image, sizeof(image.shot) + tail);
    /* Thor.980805: 再加上 shot的部分 */
    image.shot[0] = fshm->shot[0] = i;  /* 總共有幾片 ? */

    if ((fp = fopen(FN_CAMERA_LOG, "a")))
    {
        fprintf(fp, "%d/%d films, %d/%d bytes\n", i, MOVIE_MAX, tail, MOVIE_SIZE);
        fclose(fp);
    }

    exit(0);
}
