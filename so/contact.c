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

static void contact_send(CONTACT *contact);
static int contact_add(XO *xo);

static void
contact_item(
int num,
const CONTACT *contact)
{
    prints("%6d     %-*s       %-*s\n", num, IDLEN, contact->name, d_cols + 49, contact->email);
}

static int
contact_cur(
XO *xo,
int pos)
{
    const CONTACT *const contact = (const CONTACT *) xo_pool_base + pos;
    move(3 + pos - xo->top, 0);
    contact_item(pos + 1, contact);
    return XO_NONE;
}

static int
contact_body(
XO *xo)
{
    const CONTACT *contact;
    int num, max, tail;

    move(3, 0);
    clrtobot();
    max = xo->max;
    if (max <= 0)
    {
        if (vans("�n�s�W��ƶ�(y/N)�H[N] ") == 'y')
            return contact_add(xo);
        return XO_QUIT;
    }

    num = xo->top;
    contact = (const CONTACT *) xo_pool_base + num;
    tail = num + XO_TALL;
    max = BMIN(max, tail);

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
    if (vget(B_LINES_REF, 0, "�W�١G", contact->name, sizeof(contact->name), echo)
        && vget(B_LINES_REF, 0, "e-mail address�G", contact->email, sizeof(contact->email), echo))
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
        return XO_INIT;
    }
    return XO_HEAD;
}

static int
contact_delete(
XO *xo,
int pos)
{

    if (vans(msg_del_ny) == 'y')
    {
        if (!rec_del(xo->dir, sizeof(CONTACT), pos, NULL, NULL))
        {
            return XO_LOAD;
        }
    }
    return XO_FOOT;
}


static int
contact_change(
XO *xo,
int pos)
{
    CONTACT *contact, mate;

    contact = (CONTACT *) xo_pool_base + pos;

    mate = *contact;
    contact_edit(contact, GCARRY);
    if (memcmp(contact, &mate, sizeof(CONTACT)))
    {
        rec_put(xo->dir, contact, sizeof(CONTACT), pos);
        return XR_FOOT + XO_CUR;
    }

    return XO_FOOT;
}

static int
contact_help(
XO *xo)
{
    film_out(FILM_CONTACT, -1);
    return XO_HEAD;
}


static int
contact_mail(
XO *xo,
int pos)
{
    CONTACT *contact;

    contact = (CONTACT *) xo_pool_base + pos;
    contact_send(contact);
    return XO_INIT;
}

static void
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
    else if (vget(21, 0, "�D  �D�G", ve_title, TTLEN + 1, DOECHO))
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


KeyFuncList contact_cb =
{
    {XO_INIT, {contact_init}},
    {XO_LOAD, {contact_load}},
    {XO_HEAD, {contact_head}},
    {XO_BODY, {contact_body}},
    {XO_CUR | XO_POSF, {.posf = contact_cur}},

    {Ctrl('P'), {contact_add}},
    {'m' | XO_POSF, {.posf = contact_mail}},
    {'r' | XO_POSF, {.posf = contact_mail}},
    {'c' | XO_POSF, {.posf = contact_change}},
    {'s', {xo_cb_init}},
    {'d' | XO_POSF, {.posf = contact_delete}},
    {'h', {contact_help}}
};

int
Contact(void)
{
    DL_HOLD;
    XO *xo, *last;
    char fpath[80];

    last = xz[XZ_OTHER - XO_ZONE].xo;  /* record */

    utmp_mode(M_OMENU);
    usr_fpath(fpath, cuser.userid, "contact");
    xz[XZ_OTHER - XO_ZONE].xo = xo = xo_new(fpath);
    xo->cb = contact_cb;
    xo->recsiz = sizeof(CONTACT);
    xo->pos = 0;
    xover(XZ_OTHER);
    free(xo);

    xz[XZ_OTHER - XO_ZONE].xo = last;  /* restore */
    return DL_RELEASE(0);
}



