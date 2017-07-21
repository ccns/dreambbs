/*-------------------------------------------------------*/
/* dictd.c    ( YZU WindTOPBBS Ver 3.10 )                */
/*-------------------------------------------------------*/
/* author : statue.bbs@bbs.yzu.edu.tw			 */
/* target : 字典                                         */
/* create : 01/11/18                                     */
/* update : 01/11/18                                     */
/*-------------------------------------------------------*/

#include "bbs.h"

int
main_dictd()
{
	char word[80];
	char tmp[80];
	char fname[80];

	while (1)
	{
		clear();
		move(0, 23);
		outs("\033[1;37;44m◎ 風之塔中英日多用途字典 ◎\033[m");
		move(1, 0);
		outs("此字典來源為 FreeBSD 的 dict-database。字典必須輸入完整單字或片語。\n");
		outs("英漢 dreye      107366 headwords.\n");
		outs("英漢 cdict      59423 headwords.\n");
		outs("英漢 pydict     177751 headwords.\n");
		outs("英漢 moecomp    67263 headwords.\n");
		outs("英漢 netterm    6477 headwords.\n");
		outs("漢英 cedict     http://www.mandarintools.com/cedict.html.\n");
		outs("漢漢 moedict    國語辭典 民國八十七年四月版.\n");
		outs("日英 edict      Japanese-English Dictionary file.\n");
		outs("英英 web1913    Webster's Revised Unabridged Dictionary (1913)\n");
		outs("英英 wn         WordNet (r) 1.6\n");
		outs("英英 gazetteer  U.S. Gazetteer (1990)\n");
		outs("英英 jargon     Jargon File (4.2.3, 23 NOV 2000)\n");
		outs("英英 foldoc     The Free On-line Dictionary of Computing (13 Mar 01)\n");
		outs("英英 elements   Elements database 20001107\n");
		outs("英英 easton     Easton's 1897 Bible Dictionary\n");
		outs("英英 hitchcock  Hitchcock's Bible Names Dictionary (late 1800's)\n");
		outs("英英 vera       V.E.R.A. -- Virtual Entity of Relevant Acronyms 13 March 2001\n");
		outs("英英 devils     THE DEVIL'S DICTIONARY ((C)1911 Released April 15 1993)\n");
		outs("英英 world95    The CIA World Factbook (1995)\n");
		outs("作者: Shen Chuan-Hsing <statue.bbs@bbs.yzu.edu.tw>\n");

		if (!vget(22, 0, "word: ", word, 30, LCECHO))
			return 0;
		if (strchr(word, ';') || strchr(word, '"') || strchr(word, '|') || strchr(word, '&')) /* security reason */
			return 0;

		sprintf(fname, "tmp/%s.dictd", cuser.userid);
		sprintf(tmp, "/usr/local/bin/dict -h localhost \"%s\" > %s", word, fname);
		logitfile(FN_PYDICT_LOG, "DICTD", word);
		system(tmp);
		more(fname, NULL);
		unlink(fname);
	}

	return 0;
}

int xover_dict()
{
	char word[80];
	char tmp[80];
	char fname[80];

	while (1)
	{

		if (!vget(b_lines, 0, "word: ", word, 30, LCECHO))
			return 0;
		if (strchr(word, ';') || strchr(word, '"') || strchr(word, '|') || strchr(word, '&')) /* security reason */
			return 0;
		sprintf(fname, "tmp/%s.dictd", cuser.userid);
		sprintf(tmp, "/usr/local/bin/dict -h localhost \"%s\" > %s", word, fname);
		logitfile(FN_PYDICT_LOG, "DICTD", word);
		system(tmp);
		more(fname, NULL);
		unlink(fname);
	}

	return 0;
}
