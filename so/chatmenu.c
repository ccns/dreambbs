/*-------------------------------------------------------*/
/* chatmenu.c   ( YZU_CSE WindTop BBS )                  */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : chatroom actions routines                    */
/* create : 2000/01/02                                   */
/* update : NULL                                         */
/*-------------------------------------------------------*/

#undef  MODES_C
#include "bbs.h"

static int chat_add(XO *xo);
static int mode = 0;
static int kind = 0;

static void
chat_item(
int num,
const ChatAction *chat)
{
    if (!mode)
        prints("%6d %-9s %-6s %-*.*s\n", num, chat->verb, chat->brief_desc, d_cols + 55, d_cols + 55, chat->part1_msg);
    else
        prints("%6d %-9s %-6s %-*.*s\n", num, chat->verb, chat->brief_desc, d_cols + 55, d_cols + 55, chat->part2_msg);
}

static int
chat_cur(
XO *xo,
int pos)
{
    const ChatAction *const chat = (const ChatAction *) xo_pool_base + pos;
    move(3 + pos - xo->top, 0);
    chat_item(pos + 1, chat);
    return XO_NONE;
}

static int
chat_body(
XO *xo)
{
    const ChatAction *chat;
    int num, max, tail;

    move(3, 0);
    clrtobot();
    max = xo->max;
    if (max <= 0)
    {
        outs("\n《" CHATROOMNAME "動詞》目前沒有資料\n");
        outs("\n  (^P)新增資料\n");
        return XO_NONE;
    }

    num = xo->top;
    chat = (const ChatAction *) xo_pool_base + num;
    tail = num + XO_TALL;
    max = BMIN(max, tail);

    do
    {
        chat_item(++num, chat++);
    }
    while (num < max);

    return XO_NONE;
}


static int
chat_head(
XO *xo)
{
    const char *title = NULL;
    switch (kind)
    {
    case 0:
        title = CHATROOMNAME "動詞一";
        break;
    case 1:
        title = CHATROOMNAME "動詞二";
        break;
    case 2:
        title = CHATROOMNAME "動詞三";
        break;
    case 3:
        title = CHATROOMNAME "動詞四";
        break;
    case 4:
        title = CHATROOMNAME "動詞五 - 個人類";
        break;
    }

    vs_head(title, str_site);
    prints(NECK_CHATMENU, d_cols, (mode == 0) ? "訊息一" : "訊息二");
    return chat_body(xo);
}


static int
chat_load(
XO *xo)
{
    xo_load(xo, sizeof(ChatAction));
    return chat_body(xo);
}


static int
chat_init(
XO *xo)
{
    xo_load(xo, sizeof(ChatAction));
    return chat_head(xo);
}


static int
chat_edit(
ChatAction *chat,
int echo)
{
    if (echo == DOECHO)
        memset(chat, 0, sizeof(ChatAction));
    if (vget(B_LINES_REF, 0, "動詞：", chat->verb, sizeof(chat->verb), echo)
        && vget(B_LINES_REF, 0, "中文解釋：", chat->brief_desc, sizeof(chat->brief_desc), echo))
    {
        vget(B_LINES_REF, 0, "訊息一：", chat->part1_msg, sizeof(chat->part1_msg), echo);
        vget(B_LINES_REF, 0, "訊息二：", chat->part2_msg, sizeof(chat->part2_msg), echo);
        return 1;
    }
    else
        return 0;
}


static int
chat_add(
XO *xo)
{
    ChatAction chat;

    if (chat_edit(&chat, DOECHO))
    {
        rec_add(xo->dir, &chat, sizeof(ChatAction));
        xo->pos = XO_TAIL;
        xo_load(xo, sizeof(ChatAction));
    }
    return XO_HEAD;
}

static int
chat_delete(
XO *xo,
int pos)
{

    if (vans(msg_del_ny) == 'y')
    {
        if (!rec_del(xo->dir, sizeof(ChatAction), pos, NULL, NULL))
        {
            return XO_LOAD;
        }
    }
    return XO_FOOT;
}


static int
chat_change(
XO *xo,
int pos)
{
    ChatAction *chat, mate;

    chat = (ChatAction *) xo_pool_base + pos;

    mate = *chat;
    chat_edit(chat, GCARRY);
    if (memcmp(chat, &mate, sizeof(ChatAction)))
    {
        rec_put(xo->dir, chat, sizeof(ChatAction), pos);
        return XR_FOOT + XO_CUR;
    }

    return XO_FOOT;
}

static int
chat_help(
XO *xo)
{
    return XO_NONE;
}

static int
chat_mode(
XO *xo)
{
    mode ^= 1;
    return XO_HEAD;
}

static int
chat_move(
XO *xo,
int pos)
{
    const ChatAction *ghdr;
    char buf[80];
    int newOrder;

    ghdr = (const ChatAction *) xo_pool_base + pos;

    sprintf(buf + 5, "請輸入第 %d 選項的新位置：", pos + 1);
    if (!vget(B_LINES_REF, 0, buf + 5, buf, 5, DOECHO))
        return XO_FOOT;

    newOrder = TCLAMP(atoi(buf) - 1, 0, xo->max - 1);
    if (newOrder != pos)
    {
        const ChatAction ghdr_orig = *ghdr;
        const char *dir = xo->dir;
        if (!rec_del(dir, sizeof(ChatAction), pos, NULL, NULL))
        {
            rec_ins(dir, &ghdr_orig, sizeof(ChatAction), newOrder, 1);
            xo->pos = newOrder;
            return XO_LOAD;
        }
    }
    return XO_FOOT;
}

static int
chat_sync(
XO *xo)
{
    int fd, size;
    struct stat st;

    if ((fd = open(xo->dir, O_RDWR, 0600)) < 0)
        return 0;

    outz("★ 資料整理稽核中，請稍候 \x1b[5m...\x1b[m");
    refresh();

    if (!fstat(fd, &st) && (size = st.st_size) > 0)
    {
        int total;

        total = size / sizeof(ChatAction);
        size = total * sizeof(ChatAction);
        if (size >= sizeof(ChatAction))
            ftruncate(fd, size);
    }
    close(fd);
    return XO_INIT;
}


static int chat_kind(XO *xo);

KeyFuncList chat_cb =
{
    {XO_INIT, {chat_init}},
    {XO_LOAD, {chat_load}},
    {XO_HEAD, {chat_head}},
    {XO_BODY, {chat_body}},
    {XO_CUR | XO_POSF, {.posf = chat_cur}},

    {Ctrl('P'), {chat_add}},
    {'a', {chat_add}},
    {'r' | XO_POSF, {.posf = chat_change}},
    {'c' | XO_POSF, {.posf = chat_change}},
    {'s', {xo_cb_init}},
    {'S', {chat_sync}},
    {'f', {chat_mode}},
    {'M' | XO_POSF, {.posf = chat_move}},
    {KEY_TAB, {chat_kind}},
    {'d' | XO_POSF, {.posf = chat_delete}},
    {'h', {chat_help}}
};


static int
chat_kind(
XO *xo)
{
    char fpath[80];

    kind++;
    if (kind > 4) kind = 0;
    switch (kind)
    {
    case 0:
        sprintf(fpath, FN_CHAT_PARTY_DB);
        break;
    case 1:
        sprintf(fpath, FN_CHAT_SPEAK_DB);
        break;
    case 2:
        sprintf(fpath, FN_CHAT_CONDITION_DB);
        break;
    case 3:
        sprintf(fpath, FN_CHAT_PARTY2_DB);
        break;
    case 4:
        sprintf(fpath, FN_CHAT_PERSON_DB);
        break;
    }
    free(xz[XZ_OTHER - XO_ZONE].xo);
    xz[XZ_OTHER - XO_ZONE].xo = xo_new(fpath);
    xz[XZ_OTHER - XO_ZONE].xo->cb = chat_cb;
    xz[XZ_OTHER - XO_ZONE].xo->recsiz = sizeof(ChatAction);
    xz[XZ_OTHER - XO_ZONE].xo->pos = 0;
    return XO_INIT;
}

int
Chatmenu(void)
{
    DL_HOLD;
    char fpath[64];
    XO *xx, *last;

    last = xz[XZ_OTHER - XO_ZONE].xo;  /* record */

    utmp_mode(M_OMENU);
    sprintf(fpath, FN_CHAT_PARTY_DB);
    kind = 0;
    xz[XZ_OTHER - XO_ZONE].xo = xx = xo_new(fpath);
    xx->cb = chat_cb;
    xx->recsiz = sizeof(ChatAction);
    xx->pos = 0;
    xover(XZ_OTHER);
    free(xx);

    xz[XZ_OTHER - XO_ZONE].xo = last;  /* restore */
    return DL_RELEASE(0);
}



