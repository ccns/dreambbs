/*-------------------------------------------------------*/
/* contact.c   ( YZU_CSE Train BBS )                     */
/*-------------------------------------------------------*/
/* author : Jerics.bbs@bbs.yzu.edu.tw                    */
/* target : contacts list                                */
/* create : 2000/01/12                                   */
/* update : NULL                                         */
/*-------------------------------------------------------*/

#undef  MODES_C
#include "bbs.h"

extern XZ xz[];

static int contact_add(XO *xo);
void contact_send(CONTACT *contact);

static void
contact_item(
int num,
CONTACT *contact)
{
    prints("%6d     %-13s      %-*s\n", num, contact->name, d_cols + 48, contact->email);
}

static int
contact_body(
XO *xo)
{
    CONTACT *contact;
    int num, max, tail;

    move(3, 0);
    clrtobot();
    max = xo->max;
    if (max <= 0)
    {
        if (vans("�n�s�W��ƶ�(Y/N)�H[N] ") == 'y')
            return contact_add(xo);
        return XO_QUIT;
    }

    contact = (CONTACT *) xo_pool;
    num = xo->top;
    tail = num + XO_TALL;
    if (max > tail)
        max = tail;

    do
    {
        contact_item(++num, contact++);
    }
    while (num < max);

    return XO_NONE;
}


static int
contact_head(
XO *xo)
{
    vs_head("�p���W��", str_site);
    prints(NECK_CONTACT, d_cols, "");
    return contact_body(xo);
}


static int
contact_load(
XO *xo)
{
    xo_load(xo, sizeof(CONTACT));
    return contact_body(xo);
}


static int
contact_init(
XO *xo)
{
    xo_load(xo, sizeof(CONTACT));
    return contact_head(xo);
}


static int
contact_edit(
CONTACT *contact,
int echo)
{
    if (echo == DOECHO)
        memset(contact, 0, sizeof(CONTACT));
    if (vget(b_lines, 0, "�W�١G", contact->name, sizeof(contact->name), echo)
        && vget(b_lines, 0, "e-mail address�G", contact->email, sizeof(contact->email), echo))
        return 1;
    else
        return 0;
}


static int
contact_add(
XO *xo)
{
    CONTACT contact;
    if (xo->max >= MAX_CONTACT)
        vmsg("�A���p���W��w��F�W��!!");
    else if (contact_edit(&contact, DOECHO))
    {
        rec_add(xo->dir, &contact, sizeof(CONTACT));
        xo->pos = XO_TAIL /* xo->max */ ;
        return contact_init(xo);
    }
    return contact_head(xo);
}

static int
contact_delete(
XO *xo)
{

    if (vans(msg_del_ny) == 'y')
    {
        if (!rec_del(xo->dir, sizeof(CONTACT), xo->pos, NULL, NULL))
        {
            return contact_load(xo);
        }
    }
    return XO_FOOT;
}


static int
contact_change(
XO *xo)
{
    CONTACT *contact, mate;
    int pos, cur;

    pos = xo->pos;
    cur = pos - xo->top;
    contact = (CONTACT *) xo_pool + cur;

    mate = *contact;
    contact_edit(contact, GCARRY);
    if (memcmp(contact, &mate, sizeof(CONTACT)))
    {
        rec_put(xo->dir, contact, sizeof(CONTACT), pos);
        move(3 + cur, 0);
        contact_item(++pos, contact);
    }

    return XO_FOOT;
}

static int
contact_help(
XO *xo)
{
    film_out(FILM_CONTACT, -1);
    return contact_head(xo);
}


static int
contact_mail(
XO *xo)
{
    int pos, cur;
    CONTACT *contact;

    pos = xo->pos;
    cur = pos - xo->top;
    contact = (CONTACT *) xo_pool + cur;
    contact_send(contact);
    return contact_init(xo);
}

void
contact_send(
CONTACT *contact)
{
    if (bbsothermode & OTHERSTAT_EDITING)
    {
        vmsg("�A�٦��ɮ��٨S�s���@�I");
        return;
    }

    if (not_addr(contact->email))
        vmsg("E-mail �����T!!");
    else if (HAS_PERM(PERM_DENYMAIL))
        vmsg("�A���H�c�Q��!!");
    else if (vget(21, 0, "�D  �D�G", ve_title, TTLEN, DOECHO))
    {
        const char *msg;
        switch (mail_send(contact->email, ve_title))
        {
        case - 1:
            msg = err_uid;
            break;

        case - 2:
            msg = msg_cancel;
            break;

        case - 3: /* Thor.980707: �������p�� ?*/
            msg = "�ϥΪ̵L�k���H";
            break;

        default:
            msg = "�H�w�H�X";
            break;
        }
        vmsg(msg);
    }
}


KeyFunc contact_cb[] =
{
    {XO_INIT, contact_init},
    {XO_LOAD, contact_load},
    {XO_HEAD, contact_head},
    {XO_BODY, contact_body},

    {Ctrl('P'), contact_add},
    {'m', contact_mail},
    {'r', contact_mail},
    {'c', contact_change},
    {'s', contact_init},
    {'d', contact_delete},
    {'h', contact_help}
};

int
Contact(void)
{
    XO *xo;
    char fpath[80];
    utmp_mode(M_OMENU);
    usr_fpath(fpath, cuser.userid, "contact");
    xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
    xz[XZ_OTHER - XO_ZONE].cb = contact_cb;
    xo->pos = 0;
    xover(XZ_OTHER);
    free(xo);
    return 0;
}



