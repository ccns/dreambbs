
#include "bbs.h"

static char *kmail;
static int total;
static FILE *flog;

static void
reaper(
char *fpath,
char *lowid)
{
	int fd;

	char buf[256];
	ACCT acct;

	sprintf(buf, "%s/.ACCT", fpath);
	fd = open(buf, O_RDONLY, 0);
	if (fd < 0)
		return;

	if (read(fd, &acct, sizeof(acct)) != sizeof(acct))
	{
		close(fd);
		return;
	}
	close(fd);

	if (!strcmp(acct.email, kmail))
	{
		fprintf(flog, "%-13s\n", acct.userid);
		total++;
	}
}

static void
traverse(
char *fpath)
{
	DIR *dirp;
	struct dirent *de;
	char *fname, *str;

	if (!(dirp = opendir(fpath)))
	{
		return;
	}
	for (str = fpath; *str; str++);
	*str++ = '/';

	while ((de = readdir(dirp)))
	{
		fname = de->d_name;
		if (fname[0] > ' ' && fname[0] != '.')
		{
			strcpy(str, fname);
			reaper(fpath, fname);
		}
	}
	closedir(dirp);
}

int
same_mail(
char *mail)
{
	int ch;
	char *fname, fpath[256];
	kmail = mail;
	total = 0;

	flog = fopen(FN_SAMEEMAIL_LOG, "w");
	if (flog == NULL)
		return 0;

	strcpy(fname = fpath, BBSHOME"/usr/@");
	fname = (char *) strchr(fname, '@');

	for (ch = 'a'; ch <= 'z'; ch++)
	{
		fname[0] = ch;
		fname[1] = '\0';
		traverse(fpath);
	}
	fclose(flog);
	return total;
}
