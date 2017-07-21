/*-------------------------------------------------------*/
/* so/qkmj.c   ( YZU_CSE WindTop BBS )			 */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw			 */
/* target : qkmj					 */
/* create : 2003/08/02					 */
/* update : 2003/08/02					 */
/*-------------------------------------------------------*/

#undef	_MODES_C_
#include "bbs.h"

int
main_qkmj()
{

	FILE *fp;
	char buf[256];
	int size, fd;

	fd_set rd;
	struct timeval to;
	int nfds;

	vmsg("光波傳送中...");
	logitfile(FN_BBSNET_LOG, "< QKMJ >", "QKMJ");
	cutmp->ufo |= UFO_NET;
	clear();
	move(0, 0);
	outs("正連入主機...\n");
	//fp = popen("telnet -l qkmj -X sra localhost 2323","r+");
	fp = popen("env HOME=/home/bbs TERM=vt100 /usr/local/libexec/qkmj95p5-freebsd", "r+");

	if (fp)
	{
		fd = fileno(fp);
		while (1)
		{
			FD_ZERO(&rd);
			FD_SET(fd, &rd);
			FD_SET(0, &rd);
			to.tv_sec = 10;
			to.tv_usec = 0;

			nfds = fd;
			nfds = select(nfds + 1, &rd, NULL, NULL, &to);
			if (nfds <= 0)
				continue;
			if (FD_ISSET(0, &rd))
			{
				size = read(0, buf, 256);
				if (size)
				{
					write(fd, buf, size);
					continue;
				}
				else
					break;
			}
			else if (FD_ISSET(fd, &rd))
			{
				size = read(fd, buf, 256);
				if (size)
				{
					write(0, buf, size);
					continue;
				}
				else
					break;
			}
		}
		close(fd);
		pclose(fp);
		cutmp->ufo &= ~UFO_NET;
	}
	else
		vmsg("登入主機失敗...");

	return XO_HEAD;
}




