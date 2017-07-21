void
clrchyiuan(i, j)
int i, j;
{
	while (i <= j)
	{
		move(i, 0);
		clrtoeol();
		i++;
	}
}

int
show_file(char *filename, int y, int lines, int mode)
{
	FILE *fp;
	char buf[256];
	clrchyiuan(y, y + lines);
	move(y, 0);
	if ((fp = fopen(filename, "r")))
	{
		while (fgets(buf, 256, fp) && lines--)
			outs(buf);
		fclose(fp);
	}
	else
		return 0;

	return 1;
}

void
usercomplete(msg, buf)
char *msg, *buf;
{
	vget(1, 0, msg, buf, IDLEN + 1, GET_USER);
}

char *
Cdate(chrono)
time_t *chrono;
{
	struct tm *ptime;

	ptime = localtime(chrono);
	/* Thor.990329: y2k */
	sprintf(Bdate, "%02d/%02d/%02d",
			ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);
	return Bdate;
}
