#include "bbs.h"
#include "gamef.c"
#define SWAP(x, y) do { int temp=(x); (x)=(y); (y)=temp; } while (0)

/* 黑傑克遊戲 */

int cuser_money = 1000; //一開始每人1000元現金

static void show_money(int m)
{
    move(19, 0);
    clrtoeol();
    prints("\x1b[1;37;44m你現有現金: \x1b[36m%-18d\x1b[37m押注金額: \x1b[36m%-20d\x1b[m",
           cuser_money, m);
}

static int print_card(int card, int x, int y)
{
    static const char *const flower[4] = {"Ｓ", "Ｈ", "Ｄ", "Ｃ"};
    static const char *const poker[52] = {"Ａ", "Ａ", "Ａ", "Ａ", "２", "２", "２", "２", "３", "３", "３", "３",
                                          "４", "４", "４", "４", "５", "５", "５", "５", "６", "６", "６", "６",
                                          "７", "７", "７", "７", "８", "８", "８", "８", "９", "９", "９", "９",
                                          "10", "10", "10", "10", "Ｊ", "Ｊ", "Ｊ", "Ｊ", "Ｑ", "Ｑ", "Ｑ", "Ｑ",
                                          "Ｋ", "Ｋ", "Ｋ", "Ｋ"
                                         };

    move(x, y);     prints("╭───╮");
    move(x + 1, y); prints("│%s    │", poker[card]);
    move(x + 2, y); prints("│%s    │", flower[card%4]);
    move(x + 3, y); prints("│      │");
    move(x + 4, y); prints("│      │");
    move(x + 5, y); prints("│      │");
    move(x + 6, y); prints("╰───╯");
    return 0;
}


int
BlackJack(void)
{
    DL_HOLD;
    char buf[20];
    static const int num[52] = {11, 11, 11, 11, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6,
                                7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10,
                                10, 10, 10, 10, 10, 10, 10, 10
                               };
    int cardlist[52];
    int i, m, tmp = 0, ch = 0, flag;
    int win = 2, win_jack = 5; /* win 為贏時的倍率, win_jack 為前兩張就 21 點倍率 */
    int six = 10, seven = 20, aj = 10, super_jack = 20; /* 777, A+J, spade A+J 的倍率 */
    int host_count, guest_count, card_count, A_count, AA_count;
    int host_point, guest_point, mov_y;
    int host_card[12], guest_card[12];
    int money;

//  int CHEAT=0; /* 作弊參數, 1 就作弊, 0 就不作 */
    time_t now = time(0);
    time(&now);
    srand(time(NULL));
    if (cuser_money <= 0) cuser_money = 1000;
    while (-1)
    {
        memset(cardlist, 0, sizeof(cardlist));
        memset(host_card, 0, sizeof(host_card));
        memset(guest_card, 0, sizeof(guest_card));

        host_count = 2; guest_count = 1; card_count = 3; A_count = 0; AA_count = 0;
        host_point = 0; guest_point = 0; mov_y = 4; flag = 0;
        clear();
        do
        {
            show_money(money = 0);
            getdata(21, 0, "要下注多少呢(上限250000)? 按 Enter 離開>", buf, 7, DOECHO, 0);
            money = atoi(buf);
            if (!buf[0]) return DL_RELEASE(0);
            if (money > cuser_money) return DL_RELEASE(0);
        }
        while ((money < 1) || (money > 250000));
        cuser_money = cuser_money - money;
        clear();
        move(2, 0); prints("(按 y 續牌，n 不續牌，d double)");
        move(0, 0); clrtoeol(); prints("您還有 \x1b[1;44;33m%d\x1b[m 金幣", cuser_money);

//重寫發牌演算法  by jerics
        for (i = 0; i < 52; i++)
            cardlist[i] = i;
        for (i = 0; i < 52; i++)
            SWAP(cardlist[i], cardlist[rand()%52]);

#if 0
        for (i = 0; i <= 51; i++)
        {
            m = 0;
            do
            {
                j = abs(cuser_money + rand()) % 52;
                if (cardlist[j] == 0)
                {
                    cardlist[j] = i;
                    m = 1;
                }
            }
            while (m == 0);
        }
#endif

    /*  if (money>=20000) CHEAT=1;
        if (CHEAT==1) {
            if (cardlist[1]<=3) {
                SWAP(cardlist[50], cardlist[1]);
            }
        } */                            /* 作弊碼 */

        host_card[0] = cardlist[0];
        if (host_card[0] < 4)AA_count++;
        guest_card[0] = cardlist[1];

        if (guest_card[0] < 4)A_count++;
        host_card[1] = cardlist[2];
        if (host_card[1] < 4)AA_count++; /* 發前三張牌 */

        move(5, 0);  prints("╭───╮");
        move(6, 0);  prints("│      │");
        move(7, 0);  prints("│      │");
        move(8, 0);  prints("│      │");
        move(9, 0);  prints("│      │");
        move(10, 0); prints("│      │");
        move(11, 0); prints("╰───╯");
        print_card(host_card[1], 5, 4);
        print_card(guest_card[0], 15, 0);  /* 印出前三張牌 */

        host_point = num[host_card[1]];
        guest_point = num[guest_card[0]];

        do
        {
            m = 1;
            guest_card[guest_count] = cardlist[card_count];
            if (guest_card[guest_count] < 4)A_count++;
            print_card(guest_card[guest_count], 15, mov_y);
            guest_point += num[guest_card[guest_count]];

            if ((guest_card[0] >= 24 && guest_card[0] <= 27) && (guest_card[1] >= 24 && guest_card[1] <= 27) && (guest_card[2] >= 24 && guest_card[2] <= 27))
            {
                move(18, 3); prints("\x1b[1;41;33m     ７７７     \x1b[m");
                move(3, 0); prints("\x1b[1;41;33m７７７ !!! 得獎金 %d 火車幣\x1b[m", money*seven);
                cuser_money += (money * seven);
//              inmoney(money*seven);
                game_log(2, "中了 \x1b[1;33m%d\x1b[m 金幣的 \x1b[1;31m  ７７７   \x1b[m",
                         money*seven);
                pressanykey("您還有 \x1b[1;44;33m%d\x1b[m 金幣", cuser_money);
                flag = 1; m = 0;
            }

            if ((guest_card[0] == 40 && guest_card[1] == 0) || (guest_card[0] == 0 && guest_card[1] == 40))
            {
                move(18, 3); prints("\x1b[1;41;33m 超級正統 BLACK JACK  \x1b[m");
                move(3, 0); prints("\x1b[1;41;33m超級正統 BLACK JACK !!! 得獎金 %d 金幣\x1b[m", money*super_jack);
                cuser_money += (money * super_jack);
                game_log(2, "中了 \x1b[1;33m%d\x1b[m 金幣的 \x1b[1;41;33m 正統 ＡＪ \x1b[m",
                         money*super_jack);
                pressanykey("您還有 \x1b[1;44;33m%d\x1b[m 金幣", cuser_money);
                flag = 1; m = 0;
            }

            if ((guest_card[0] <= 3 && guest_card[0] >= 0) && (guest_card[1] <= 43 && guest_card[1] >= 40))tmp = 1;

            if ((tmp == 1) || ((guest_card[1] <= 3 && guest_card[1] >= 0) && (guest_card[0] <= 43 && guest_card[0] >= 40)))
            {
                move(18, 3); prints("\x1b[1;41;33m SUPER BLACK JACK  \x1b[m");
                move(3, 0); prints("\x1b[1;41;33mSUPER BLACK JACK !!! 得獎金 %d 金幣\x1b[m", money*aj);
                cuser_money += (money * aj);
//              inmoney(money*aj);
                game_log(2, "中了 \x1b[1;33m%d\x1b[m 元的 \x1b[1;44;33m SuperＡＪ \x1b[m", money*aj);
                pressanykey("您還有 \x1b[1;44;33m%d\x1b[m 金幣", cuser_money);
                flag = 1; m = 0; tmp = 0;
            }

            if (guest_point == 21 && guest_count == 1)
            {
                move(18, 3); prints("\x1b[1;41;33m  BLACK JACK  \x1b[m");
                move(3, 0); prints("\x1b[1;41;33mBLACK JACK !!!\x1b[44m 得獎金 %d 金幣\x1b[m", money*win_jack);
                cuser_money += (money * win_jack);
//              inmoney(money*win_jack);
                move(0, 0); clrtoeol();//prints("您還有 \x1b[1;44;33m%d\x1b[m 金幣", cuser.money);
                if (money*win_jack >= 500000)
                {
                    game_log(2, "中了 \x1b[1;33m%d\x1b[m 金幣的 \x1b[1;47;30m BlackJack \x1b[m", money*win_jack);
                }

                pressanykey("您還有 \x1b[1;44;33m%d\x1b[m 金幣", cuser_money);
                flag = 1; m = 0;
            }                        /* 前兩張就 21 點 */

            if (guest_point > 21)
            {
                if (A_count > 0) { guest_point -= 10; A_count--; }
            }
            move(12, 0); clrtoeol(); prints("\x1b[1;32m點數: \x1b[33m%d\x1b[m", host_point);
            move(14, 0); clrtoeol(); prints("\x1b[1;32m點數: \x1b[33m%d\x1b[m", guest_point);
            if (guest_point > 21)
            {
                pressanykey("  爆掉啦~~~  ");
                flag = 1; m = 0;
            }

            if ((guest_count == 5) && (flag == 0))
            {
                move(18, 3); prints("\x1b[1;41;33m            過六關            \x1b[m");
                move(3, 0); prints("\x1b[1;41;33m過六關 !!! 得獎金 %d 金幣\x1b[m", money*six);
                cuser_money += (money * six);
//              inmoney(money*six);
//              inexp(ba*5);
                game_log(2, "中了 \x1b[1;33m%d\x1b[m 元的 \x1b[1;44;33m  過六關   \x1b[m", money*six);
                pressanykey("您還有 %d 金幣", cuser_money);
                flag = 1; m = 0;
//              return DL_RELEASE(0);
            }

            guest_count++;
            card_count++;
            mov_y += 4;

            do
            {
                if (ch == 'd')m = 0;
                if (m != 0) ch = vkey();
            }
            while (ch != 'y' && ch != 'n' && ch != 'd' && m != 0); /* 抓 key */

            if (ch == 'd' && m != 0 && guest_count == 2)
            {
                if (cuser_money >= money)
                {
                    cuser_money -= money;
//                  demoney(money);
                    money *= 2;
                }
                else ch = 'n';
                move(0, 0); clrtoeol(); prints("您還有 \x1b[1;44;33m%d\x1b[m 金幣", cuser_money);
            }                                      /* double */

            if (ch == 'd' && guest_count > 2)ch = 'n';
            if (guest_point == 21)ch = 'n';
        }
        while (ch != 'n' && m != 0);
        mov_y = 8;

        print_card(host_card[0], 5, 0);
        print_card(host_card[1], 5, 4);
        host_point += num[host_card[0]];
        if (!flag)
        {
            do
            {
                if (host_point < guest_point)
                {
                    host_card[host_count] = cardlist[card_count];
                    print_card(host_card[host_count], 5, mov_y);
                    if (host_card[host_count] < 4)AA_count++;
                    host_point += num[host_card[host_count]];
                }
                if (host_point > 21)
                {
                    if (AA_count > 0) { host_point -= 10; AA_count--; }
                }
                move(12, 0); clrtoeol(); prints("\x1b[1;32m點數: \x1b[33m%d\x1b[m", host_point);
                move(14, 0); clrtoeol(); prints("\x1b[1;32m點數: \x1b[33m%d\x1b[m", guest_point);
                if (host_point > 21)
                {
                    move(14, 0); clrtoeol(); prints("\x1b[1;32m點數: \x1b[33m%d \x1b[1;41;33m WINNER \x1b[m", guest_point);
                    cuser_money += (money * win);
//                  inmoney(money*win);
                    move(0, 0); clrtoeol(); prints("您還有 \x1b[1;44;33m%d\x1b[m 金幣", cuser_money);
                    pressanykey("\x1b[1;44;33m你贏了~~~~ 得獎金 %d 金幣\x1b[m", money*win);
                    flag = 1;
                }
                host_count++;
                card_count++;
                mov_y += 4;
            }
            while (host_point < guest_point);
            if (!flag) pressanykey("你輸了~~~~ 金幣沒收!");
        }
    }
}
