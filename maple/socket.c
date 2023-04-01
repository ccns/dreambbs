#include "bbs.h"

#include <netinet/tcp.h>

int
Get_Socket(  /* site for hostname, sock for port & socket */
    const char *site,
    int *sock)
{
    static const unsigned int sec_timeout = 5; /* IID.2023-02-21: TCP-based timeout for connect(). Unit: Seconds */
    struct addrinfo hints = {0};
    struct addrinfo *hosts;
    char port_str[12];

    /* Getting remote-site data */

    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_ADDRCONFIG | AI_NUMERICSERV;
    sprintf(port_str, "%d", *sock);
    if (getaddrinfo(site, port_str, &hints, &hosts))
        return -1;

    /* Getting a socket */

    for (struct addrinfo *host = hosts; host; host = host->ai_next)
    {
        if ((*sock = socket(host->ai_family, host->ai_socktype, 0)) < 0)
        {
            continue;
        }

#ifdef  SET_ALARM
        signal(SIGALRM, timeout);
        alarm(SET_ALARM);
#else
        setsockopt(*sock, IPPROTO_TCP, TCP_USER_TIMEOUT, &TEMPLVAL(unsigned int, {1000 * sec_timeout}), sizeof(unsigned int));
#endif

        /* perform connecting */

        if (connect(*sock, host->ai_addr, host->ai_addrlen) < 0)
        {
            close(*sock);

#ifdef  SET_ALARM
            init_alarm();
#endif

            continue;
        }

        /* Success */

#ifdef  SET_ALARM
        init_alarm();
#else
        setsockopt(*sock, IPPROTO_TCP, TCP_USER_TIMEOUT, &TEMPLVAL(unsigned int, {0}), sizeof(unsigned int));
#endif


        freeaddrinfo(hosts);
        return 0;
    }

#ifdef  SET_ALARM
    init_alarm();
#endif

    freeaddrinfo(hosts);
    return -1;
}



int
POP3_Check(
    const char *site, const char *account, const char *passwd)
{
    FILE *fsock = NULL;
    int sock=110;
    int res = 5; // default error value
    char buf[512];

    if (Get_Socket(site, &sock))
        goto socket_error;

    if (!(fsock = fdopen(sock, "r+")))
    {
        close(sock);
        goto socket_error;
    }

    for (int step = 1;; ++step)
    {
        switch (step)
        {
            case 1:             /* Welcome Message */
                fgets(buf, 512, fsock);
                break;

            case 2:             /* Verify Account */
                fprintf(fsock, "user %s\r\n", account);
                fflush(fsock);
                fgets(buf, 512, fsock);
                break;

            case 3:             /* Verify Password */
                fprintf(fsock, "pass %s\r\n", passwd);
                fflush(fsock);
                fgets(buf, 512, fsock);
                break;

            default:
            case 4:
                res = 0;
                goto quit_ok;
        }

        if (strncmp(buf, "+OK", 3) != 0 || strstr(buf, ".bbs"))
        {
            prints("遠端系統傳回錯誤訊息如下：(如不明白訊息意思，請將訊息告知站長)\n");
            prints("%s\n", buf);
            refresh();
/*          log_usies("POP3-MSG", buf, FN_VERIFY_LOG); */
            goto quit_error;
        }
    }
    /* Unreachable */

quit_ok:        /* Successful Verification */
quit_error:     /* Quit */
    fprintf(fsock, "quit\r\n");
    fclose(fsock);
    return res;

socket_error:   /* Open Socket Fail */
    prints("\n傳回錯誤值 [1]，請重試幾次看看\n");
    refresh();
    return sock;
}


int
Ext_POP3_Check(
    const char *site, const char *account, const char *passwd)
{
    FILE *fsock = NULL;
    int sock=110;
    int res = 9; // default error value

    const char *msg_step = "unknown";
    char buf[512];

    fd_set rd;
    struct timeval to;
    int nfds;
    time_t now;
    struct tm *p;


    if (Get_Socket(site, &sock))
        return 1;

    if (!(fsock = fdopen(sock, "r+")))
    {
        close(sock);
        return 1;
    }

    for (int step = 1;;)
    {
        FD_ZERO(&rd);
        FD_SET(sock, &rd);
        to.tv_sec = 0;
        to.tv_usec = 0;
        nfds = sock;

#define POP3_CHECK_WAIT_SOCK() do { \
    FD_ZERO(&rd); \
    FD_SET(sock, &rd); \
    nfds = select(nfds+1, &rd, NULL, NULL, &to); \
    if (nfds == 0) \
        break; \
} while (0)

        switch (step)
        {
        case 1:
            msg_step = "stat 1";
            for (;;)
            {
                fgets(buf, 512, fsock);
                if (strncmp(buf, "+OK", 3) != 0)
                {
                    pmsg("不支援 POP3 主機，請使用 Email 回信認證！");
                    goto quit_error;
                }
                if (step != 1)
                    goto quit_untrusted;
                step = 2;
                POP3_CHECK_WAIT_SOCK();
            }
            break;
        case 2:
            msg_step = "stat 2";
            fprintf(fsock, "pass !@#$%%^&*(\r\n");
            fflush(fsock);
            for (;;)
            {
                fgets(buf, 512, fsock);
                if (strncmp(buf, "+OK", 3) == 0)
                    goto quit_untrusted;
                if (strncmp(buf, "-ERR", 4) == 0)
                    step = 3;
                POP3_CHECK_WAIT_SOCK();
            }
            break;
        case 3:
            msg_step = "stat 3";
            fprintf(fsock, "noop\r\n");
            fflush(fsock);
            for (;;)
            {
                fgets(buf, 512, fsock);
                if (strncmp(buf, "+OK", 3) == 0)
                    goto quit_untrusted;
                if (strncmp(buf, "-ERR", 4) == 0)
                    step = 4;
                POP3_CHECK_WAIT_SOCK();
            }
            break;
        case 4:
            msg_step = "stat 4";
            fprintf(fsock, "user %s\r\n", account);
            fflush(fsock);
            for (;;)
            {
                fgets(buf, 512, fsock);
                if (strncmp(buf, "-ERR", 4) == 0)
                {
                    pmsg("無此郵件帳號，請使用 Email 回信認證！");
                    goto quit_error;
                }
                if (!(strncmp(buf, "+OK", 3) == 0 && step == 4))
                    goto quit_untrusted;
                step = 5;
                POP3_CHECK_WAIT_SOCK();
            }
            break;
        case 5:
            msg_step = "stat 5";
            fprintf(fsock, "pass %s\r\n", passwd);
            fflush(fsock);
            for (;;)
            {
                fgets(buf, 512, fsock);
                if (strncmp(buf, "-ERR", 4) == 0)
                {
                    pmsg("帳號密碼錯誤，請重新認證！");
                    goto quit_error;
                }
                if (!(strncmp(buf, "+OK", 3) == 0 && step == 5))
                    goto quit_untrusted;
                step = 6;
                POP3_CHECK_WAIT_SOCK();
            }
            break;
        case 6:
            msg_step = "stat 6";
            fprintf(fsock, "noop\r\n");
            fflush(fsock);
            for (;;)
            {
                fgets(buf, 512, fsock);
                if (!(strncmp(buf, "+OK", 3) == 0 && step == 6))
                    goto quit_untrusted;
                step = 7;
                POP3_CHECK_WAIT_SOCK();
            }
            break;
        case 7:
            msg_step = "stat 7";
            fprintf(fsock, "stat\r\n");
            fflush(fsock);
            for (;;)
            {
                fgets(buf, 512, fsock);
                if (!(strncmp(buf, "+OK", 3) == 0 && step == 7))
                    goto quit_untrusted;
                step = 8;
                POP3_CHECK_WAIT_SOCK();
            }
            break;

        default:
        case 8:
            res = 0;
            goto quit_ok;
        }
    }
#undef POP3_CHECK_WAIT_SOCK
    /* Unreachable */

quit_untrusted:
    res = 8;

    pmsg("不受信任的 POP3 主機，請使用 Email 回信認證！");
    logitfile("tmp/visor.log", site, msg_step);

    time(&now);

    p = localtime(&now);
    sprintf(buf, "%s # %02d/%02d/%02d %02d:%02d 不受信任的主機\n",
            site, p->tm_year % 100, p->tm_mon + 1, p->tm_mday,
            p->tm_hour, p->tm_min);
    f_cat(FN_ETC_UNTRUST_ACL, buf);
    // Falls through
quit_ok:
quit_error:
    fprintf(fsock, "quit\r\n");
    fclose(fsock);
    return res;

}
