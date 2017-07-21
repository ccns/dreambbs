/*-------------------------------------------------------*/
/* railway.c    ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* author : plateau@wahoo.com.tw                         */
/* modify : statue.bbs@bbs.yzu.edu.tw			 */
/* target : 字典                                         */
/* create : 01/05/06                                     */
/* update : 01/01/30                                     */
/*-------------------------------------------------------*/

#include "bbs.h"

int
main_pydict()
{
	char ans[2];
	char word[80];
	char tmp[80];
	char fname[80];

	clear();
	move(0, 23);
	outs("\033[1;37;44m◎ 風之塔英漢漢英字典 ◎\033[m");
	move(3, 0);
	outs("此字典來源為 FreeBSD 的 pyDict。\n");
	outs("英漢字典必須輸入完整單字或片語。\n");
	outs("而漢英字典則會列出相關的英文單字或片語。\n");
	outs("pydict database have 177751 headwords.\n");
	outs("作者: Shen Chuan-Hsing <statue.bbs@bbs.yzu.edu.tw>。\n");

	if (!vget(9, 0, "1)英漢字典  2)漢英字典 [q] ", ans, 3, LCECHO))
		return 0;
	if (ans[0] != '1' && ans[0] != '2')
		return 0;
	vget(10, 0, "word: ", word, 60, LCECHO);
	if (strchr(word, ';') || strchr(word, '"') || strchr(word, '|') || strchr(word, '&')) /* security reason */
		return 0;
	sprintf(fname, "tmp/%s.pydict", cuser.userid);
	if (ans[0] == '2')
		sprintf(tmp, "bin/pydict -c \"%s\" > %s", word, fname);
	else if (ans[0] == '1')
		sprintf(tmp, "bin/pydict -e \"%s\" > %s", word, fname);
	else
		return 0;
	logitfile(FN_PYDICT_LOG, "PYDICT", word);
	system(tmp);
	more(fname, NULL);
	unlink(fname);
	return 0;
}

