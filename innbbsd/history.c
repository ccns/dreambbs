/*-------------------------------------------------------*/
/* history.c    ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* target : innbbsd history                              */
/* create : 04/04/01                                     */
/* update :   /  /                                       */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "innbbsconf.h"
#include "inntobbs.h"


typedef struct
{
    time32_t htime;             /* �[�J history �ɪ��ɶ� */
    int32_t hash;               /* ���F�ֳt�j�M */
    char msgid[256];            /* message id (���] 256 �w����) */
    char board[IDLEN + 1];
    char xname[9];
}       HIS;  /* DISKDATA(raw) */


void
HISmaint(void)                  /* ���@ history �ɡA�N�L���� history �R�� */
{
    int i, fd, total;
    char fpath[64];
    time_t now;
    struct stat st;
    HIS *data, *hhead, *htail, *his;

    /* �u�O�d�̪� EXPIREDAYS �Ѫ� history */
    time(&now);
    now = time(NULL) - EXPIREDAYS * 86400;

    for (i = 0; i < 32; i++)
    {
        sprintf(fpath, "innd/history/%02d", i);

        if ((fd = open(fpath, O_RDONLY)) < 0)
            continue;

        fstat(fd, &st);
        data = (HIS *) malloc(total = st.st_size);
        total = read(fd, data, total);
        close(fd);

        hhead = data;
        htail = data + total / sizeof(HIS);
        total = 0;

        for (his = hhead; his < htail; his++)
        {
            if (his->htime > now)       /* �o�� history ���Q�� */
            {
                memcpy(hhead, his, sizeof(HIS));
                hhead++;
                total += sizeof(HIS);
            }
        }

        if ((fd = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, 0600)) >= 0)
        {
            write(fd, data, total);
            close(fd);
        }

        free(data);
    }
}


void
HISadd(                         /* �N (msgid, path, xname) ���t��O���b history �� */
    const char *msgid,
    const char *board,
    const char *xname)
{
    HIS his;
    char fpath[64];

    memset(&his, 0, sizeof(HIS));

    time32(&(his.htime));
    his.hash = str_hash(msgid, 1);
    str_scpy(his.msgid, msgid, sizeof(his.msgid));
    str_scpy(his.board, board, sizeof(his.board));
    str_scpy(his.xname, xname, sizeof(his.xname));

    /* �� msgid �N history ������ 32 ���ɮ� */
    sprintf(fpath, "innd/history/%02d", his.hash % 32U);
    rec_add(fpath, &his, sizeof(HIS));
}


int                             /* 1:�bhistory�� 0:���bhistory�� */
HISfetch(                       /* �d�� history ���Amsgid �o��h�F���� */
    const char *msgid,
    char *board,                /* �ǥX�b history �����O�����ݪO���ɦW */
    char *xname)
{
    HIS his;
    char fpath[64];
    int fd, hash;
    int rc = 0;

    /* �p�G�P�@ msgid �o��h�ܦh�ӬݪO�A����ثe�u�|�^�ǲĤ@�ӬݪO���ɦW */

    /* �� msgid ��X�b���@�� history �ɮפ� */
    hash = str_hash(msgid, 1);
    sprintf(fpath, "innd/history/%02d", hash % 32U);

    /* �h�ӥ� history �ɮפ���ݬݦ��S�� */
    if ((fd = open(fpath, O_RDONLY)) >= 0)
    {
        lseek(fd, 0, SEEK_SET);
        while (read(fd, &his, sizeof(HIS)) == sizeof(HIS))
        {
            /* �� hash ���ʲ����A�Y�ۦP�A�� msgid ������ */
            if ((hash == his.hash) && !strcmp(msgid, his.msgid))
            {
                if (board)
                    strcpy(board, his.board);
                if (xname)
                    strcpy(xname, his.xname);
                rc = 1;
                break;
            }
        }
        close(fd);
    }

    return rc;
}
