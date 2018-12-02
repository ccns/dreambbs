/*-------------------------------------------------------*/
/* rfc2047.c		( NTHU CS MapleBBS Ver 3.10 )	 */
/*-------------------------------------------------------*/
/* target : RFC 2047 QP/base64 encode			 */
/* create : 03/04/11					 */
/* update : 03/05/19					 */
/* author : PaulLiu.bbs@bbs.cis.nctu.edu.tw		 */
/*-------------------------------------------------------*/

#include <stdio.h>


/*-------------------------------------------------------*/
/* RFC2047 QP encode					 */
/*-------------------------------------------------------*/

void
output_rfc2047_qp(
    FILE* fp,
    char* prefix,
    char* str,
    char* charset,
    char* suffix
)
{
    int i, ch;
    int blank = 1;	/* 1:全由空白組成 */
    static char tbl[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    fputs(prefix, fp);

    /* 如果字串開頭有 US_ASCII printable characters，可先行輸出，這樣比較好看，也比較相容 */
    for ( i = 0; (ch = str[i]); i++)
    {
        if (ch != '=' && ch != '?' && ch != '_' && ch > '\x1f' && ch < '\x7f')
        {
            if (blank)
            {
                if (ch != ' ')
                    blank = 0;
                else if (str[i + 1] == '\0')	/* 若全是空白，最後一個要轉碼 */
                    break;
            }
            fprintf(fp, "%c", ch);
        }
        else
            break;
    }

    if (ch != '\0')	/* 如果都沒有特殊字元就結束 */
    {
        /* 開始 encode */
        fprintf(fp, "=?%s?Q?", charset);	/* 指定字集 */
        for (; (ch = str[i]); i++)
        {
            /* 如果是 non-printable 字元就要轉碼 */
            /* 範圍: '\x20' ~ '\x7e' 為 printable, 其中 =, ?, _, 空白, 為特殊符號也要轉碼 */

            if (ch == '=' || ch == '?' || ch == '_' || ch <= '\x1f' || ch >= '\x7f')
                fprintf(fp, "=%c%c", tbl[(ch >> 4) & '\x0f'], tbl[ch & '\x0f']);
            else if (ch == ' ')	/* 空白比較特殊, 轉成 '_' 或 "=20" */
                fprintf(fp, "=20");
            else
                fprintf(fp, "%c", ch);
        }
        fputs("?=", fp);
    }

    fputs(suffix, fp);
}

