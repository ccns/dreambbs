/*-------------------------------------------------------*/
/* railway.c    ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* author : plateau@wahoo.com.tw                         */
/* modify : statue.bbs@bbs.yzu.edu.tw			 */
/* target : �r��                                         */
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
	outs("\033[1;37;44m�� ������^�~�~�^�r�� ��\033[m");
	move(3, 0);
	outs("���r��ӷ��� FreeBSD �� pyDict�C\n");
	outs("�^�~�r�奲����J�����r�Τ��y�C\n");
	outs("�Ӻ~�^�r��h�|�C�X�������^���r�Τ��y�C\n");
	outs("pydict database have 177751 headwords.\n");
	outs("�@��: Shen Chuan-Hsing <statue.bbs@bbs.yzu.edu.tw>�C\n");

	if (!vget(9, 0, "1)�^�~�r��  2)�~�^�r�� [q] ", ans, 3, LCECHO))
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

