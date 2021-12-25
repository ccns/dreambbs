/*-------------------------------------------------------*/
/* util/counter.c       ( YZU WindTopBBS Ver 3.00 )      */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : 歷史軌跡的紀錄                               */
/* create : 95/03/29                                     */
/* update : 97/03/29                                     */
/*-------------------------------------------------------*/
/* syntax : counter                                      */
/*-------------------------------------------------------*/

#include "bbs.h"

COUNTER *count;

int
main(
    int argc,
    char *argv[])
{
    int fd;
    time_t now;
    struct tm ntime, *xtime, ptime;
    FILE *fp;
    char ymd[80];

    setgid(BBSGID);
    setuid(BBSUID);
    chdir(BBSHOME);

    count_init(&count);

    optind++;
    switch ((argc > 1) ? *argv[1] : 0)
    {
    case 'r':
        printf("\nhour_max_login = %d \n", count->hour_max_login);
        printf("day_max_login = %d \n", count->day_max_login);
        printf("samehour_max_login = %d \n", count->samehour_max_login);
        printf("max_regist = %d \n", count->max_regist);
        printf("cur_hour_max_login = %d \n", count->cur_hour_max_login);
        printf("cur_day_max_login = %d \n", count->cur_day_max_login);
        printf("samehour_max_time = %ld \n", (long)count->samehour_max_time);
        printf("samehour_max_login_old = %d \n", count->samehour_max_login_old);
        printf("max_regist_old = %d \n", count->max_regist_old);
        break;

    case 'w':
        {
            int opt, arg_r = 4;  // Need 4 arguments when only position arguments are given
            int sml = -1, mr = -1, mhl = -1, mdl = -1;
            while (optind < argc)
            {
                switch (opt = getopt(argc, argv, "+" "s:r:h:d"))
                {
                case -1:  // Position arguments
                    if (!(optarg = argv[optind++]))
                        break;
                    arg_r--;
                    if (!(sml >= 0))
                case 's':
                        sml = atoi(optarg);
                    else if (!(mr >= 0))
                case 'r':
                        mr = atoi(optarg);
                    else if (!(mhl >= 0))
                case 'h':
                        mhl = atoi(optarg);
                    else if (!(mdl >= 0))
                case 'd':
                        mdl = atoi(optarg);

                    if (opt != -1) arg_r = 0;  // Allow skipped arguments
                    if (atoi(optarg) >= 0)  // Arguments must >= 0
                        break;
                    // Else falls through
                default:
                    arg_r = sml = mr = mhl = mdl = -1;
                    optind = argc;  // Ignore remaining arguments
                    break;
                }
            }

            if (arg_r <= 0)
            {
                if (sml >= 0)
                    count->samehour_max_login = sml;
                if (mr >= 0)
                    count->max_regist = mr;
                if (mhl >= 0)
                    count->hour_max_login = mhl;
                if (mdl >= 0)
                    count->day_max_login = mdl;
            }
            else
            {
                fprintf(stderr, "%s w [[-s] <SML>] [[-r] <MR>] [[-h] <MHL>] [[-d] <MHD>] \n", argv[0]);
                fprintf(stderr, "SML >= 0: 同時在站內人數\n");
                fprintf(stderr, "MR >= 0: 總註冊人數\n");
                fprintf(stderr, "MHL >= 0: 單一小時上線人次\n");
                fprintf(stderr, "MHD >= 0: 單日上線人次\n");
                return 2;
            }
            break;
        }

    default:
        fp = fopen(FN_ETC_COUNTER, "a+");

        now = time(NULL);
        xtime = localtime(&now);
        ntime = *xtime;

        sprintf(ymd, "%02d/%02d/%02d",
            ntime.tm_year % 100, ntime.tm_mon + 1, ntime.tm_mday);

        now = count->samehour_max_time;
        xtime = localtime(&now);
        ptime = *xtime;

        if (count->max_regist > count->max_regist_old)
        {
            fprintf(fp, "★ 【%s】\x1b[1;32m總註冊人數\x1b[m提升到 \x1b[31;1m%d\x1b[m 人\n", ymd, count->max_regist);
            count->max_regist_old = count->max_regist;
        }

        if (count->samehour_max_login > count->samehour_max_login_old)
        {
            fprintf(fp, "◎ 【%s %02d:%02d】\x1b[32m同時在站內人數\x1b[m首次達到 \x1b[1;36m%d\x1b[m 人次\n", ymd, ptime.tm_hour, ptime.tm_min, count->samehour_max_login);
            count->samehour_max_login_old = count->samehour_max_login;
        }

        if (count->cur_hour_max_login > count->hour_max_login)
        {
            fprintf(fp, "◇ 【%s %02d】\x1b[1;32m單一小時上線人次\x1b[m首次達到 \x1b[1;35m%d\x1b[m 人次\n", ymd, ntime.tm_hour, count->cur_hour_max_login);
            count->hour_max_login = count->cur_hour_max_login;
        }
        count->cur_hour_max_login = 0;

        if (ntime.tm_hour == 0)
        {
            if (count->cur_day_max_login > count->day_max_login)
            {
                fprintf(fp, "◆ 【%s】\x1b[1;32m單日上線人次\x1b[m首次達到 \x1b[1;33m%d\x1b[m 人次\n", ymd, count->cur_day_max_login);
                count->day_max_login = count->cur_day_max_login;
            }
            count->cur_day_max_login = 0;
        }
        if ((fd = open(FN_VAR_SYSHISTORY, O_WRONLY | O_CREAT | O_TRUNC, 0600)) >= 0)
        {
            write(fd, count, sizeof(COUNTER));
            close(fd);
        }

        fclose(fp);
        break;
    }
    return 0;
}
