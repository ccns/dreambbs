/*-------------------------------------------------------*/
/* dictd.c    ( YZU WindTOPBBS Ver 3.10 )                */
/*-------------------------------------------------------*/
/* author : statue.bbs@bbs.yzu.edu.tw			 */
/* target : �r��                                         */
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
		outs("\033[1;37;44m�� �����𤤭^��h�γ~�r�� ��\033[m");
		move(1, 0);
		outs("���r��ӷ��� FreeBSD �� dict-database�C�r�奲����J�����r�Τ��y�C\n");
		outs("�^�~ dreye      107366 headwords.\n");
		outs("�^�~ cdict      59423 headwords.\n");
		outs("�^�~ pydict     177751 headwords.\n");
		outs("�^�~ moecomp    67263 headwords.\n");
		outs("�^�~ netterm    6477 headwords.\n");
		outs("�~�^ cedict     http://www.mandarintools.com/cedict.html.\n");
		outs("�~�~ moedict    ��y��� ����K�Q�C�~�|�목.\n");
		outs("��^ edict      Japanese-English Dictionary file.\n");
		outs("�^�^ web1913    Webster's Revised Unabridged Dictionary (1913)\n");
		outs("�^�^ wn         WordNet (r) 1.6\n");
		outs("�^�^ gazetteer  U.S. Gazetteer (1990)\n");
		outs("�^�^ jargon     Jargon File (4.2.3, 23 NOV 2000)\n");
		outs("�^�^ foldoc     The Free On-line Dictionary of Computing (13 Mar 01)\n");
		outs("�^�^ elements   Elements database 20001107\n");
		outs("�^�^ easton     Easton's 1897 Bible Dictionary\n");
		outs("�^�^ hitchcock  Hitchcock's Bible Names Dictionary (late 1800's)\n");
		outs("�^�^ vera       V.E.R.A. -- Virtual Entity of Relevant Acronyms 13 March 2001\n");
		outs("�^�^ devils     THE DEVIL'S DICTIONARY ((C)1911 Released April 15 1993)\n");
		outs("�^�^ world95    The CIA World Factbook (1995)\n");
		outs("�@��: Shen Chuan-Hsing <statue.bbs@bbs.yzu.edu.tw>\n");

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
