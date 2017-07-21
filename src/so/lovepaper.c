#include "bbs.h"

#define         LOVE_PAPER      FN_ETC_LOVEPAPER
#define		P_BEGIN		"@begin"
#define		L_BEGIN		(3)
#define		P_MID1		"@mid1"
#define		L_MID1		(2)
#define		P_MID2		"@mid2"
#define		L_MID2		(2)
#define		P_MID3		"@mid3"
#define		L_MID3		(2)
#define		P_END		"@end"
#define		L_END		(3)
#define		P_POEM		"@poem"

int
lovepaper()
{
	FILE *fp;
	int line, page, mode, rpage, cpage, style;
	char buf[300];
	char *header[] = {P_BEGIN, P_MID1, P_MID2, P_MID3, P_END, P_POEM};
	int  h_line[] = {L_BEGIN, L_MID1, L_MID2, L_MID3, L_END};

	srand(time(0));
	if ((fp = fopen(LOVE_PAPER, "r+")))
	{
		clear();
		mode = 0;
		style = 0;
		while (fgets(buf, sizeof(buf), fp))
		{
			if (strstr(buf, "#"))
				continue;
			switch (mode)
			{
			case 0:
				if (strstr(buf, header[style]))
				{
					if (strstr(buf, P_POEM))
					{
						mode++;
						break;
					}
					fgets(buf, sizeof(buf), fp);
					page = atoi(buf);
					rpage = rand() % page;
				}
				else
					break;
				line = 0;
				for (cpage = 0;cpage < page;)
				{
					if (!fgets(buf, sizeof(buf), fp))
						break;
					if (cpage == rpage)
						prints("%s", buf);
					line++;
					if (line >= h_line[style])
					{
						cpage++;
						line = 0;
					}
				}
				style++;
				break;
			case 1:
				prints("--\n");
				page = atoi(buf);
				rpage = rand() % page;
				line = 0;
				for (cpage = 0;cpage < page;)
				{
					if (!fgets(buf, sizeof(buf), fp))
						break;
					if (strstr(buf, "$"))
					{
						line = 1;
					}
					else if (cpage == rpage)
						prints("%s", buf);
					if (line)
					{
						cpage++;
						line = 0;
					}
				}
				break;
			}
		}
		fclose(fp);
	}
	vmsg("請按任意鍵結束！");
	return 0;
}

