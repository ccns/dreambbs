#include "bbs.h"
#include "gamef.c"
#define SWAP(x, y) do {int temp=(x); (x)=(y); (y)=temp;} while (0)

/* �³ǧJ�C�� */

int cuser_money = 1000; //�@�}�l�C�H1000���{��

void show_money(int m)
{
    move(19, 0);
    clrtoeol();
    prints("\x1b[1;37;44m�A�{���{��: \x1b[36m%-18d\x1b[37m��`���B: \x1b[36m%-20d\x1b[m",
           cuser_money, m);
}

int print_card(int card, int x, int y)
{
    const char *flower[4] = {"��", "��", "��", "��"};
    const char *poker[52] = {"��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��",
                             "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��",
                             "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��",
                             "10", "10", "10", "10", "��", "��", "��", "��", "��", "��", "��", "��",
                             "��", "��", "��", "��"
                            };

    move(x, y);     prints("�~�w�w�w��");
    move(x + 1, y); prints("�x%s    �x", poker[card]);
    move(x + 2, y); prints("�x%s    �x", flower[card%4]);
    move(x + 3, y); prints("�x      �x");
    move(x + 4, y); prints("�x      �x");
    move(x + 5, y); prints("�x      �x");
    move(x + 6, y); prints("���w�w�w��");
    return 0;
}


int
BlackJack(void)
{
    char buf[20];
    int  num[52] = {11, 11, 11, 11, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6,
                     7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10,
                     10, 10, 10, 10, 10, 10, 10, 10
                   };
    int cardlist[52];
    int i, m, tmp = 0, ch = 0, flag;
    int win = 2, win_jack = 5; /* win ��Ĺ�ɪ����v, win_jack ���e��i�N 21 �I���v */
    int six = 10, seven = 20, aj = 10, super_jack = 20; /* 777, A+J, spade A+J �����v */
    int host_count, guest_count, card_count, A_count, AA_count;
    int host_point, guest_point, mov_y;
    int host_card[12], guest_card[12];
    int money;

//  int CHEAT=0; /* �@���Ѽ�, 1 �N�@��, 0 �N���@ */
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
            getdata(21, 0, "�n�U�`�h�֩O(�W��250000)? �� Enter ���}>", buf, 7, DOECHO, 0);
            money = atoi(buf);
            if (!buf[0]) return 0;
            if (money > cuser_money) return 0;
        }
        while ((money < 1) || (money > 250000));
        cuser_money = cuser_money - money;
        clear();
        move(2, 0); prints("(�� y ��P�An ����P�Ad double)");
        move(0, 0); clrtoeol(); prints("�z�٦� \x1b[1;44;33m%d\x1b[m ����", cuser_money);

//���g�o�P�t��k  by jerics
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
        if (CHEAT==1){
            if (cardlist[1]<=3){
                SWAP(cardlist[50], cardlist[1]);
            }
        } */                            /* �@���X */

        host_card[0] = cardlist[0];
        if (host_card[0] < 4)AA_count++;
        guest_card[0] = cardlist[1];

        if (guest_card[0] < 4)A_count++;
        host_card[1] = cardlist[2];
        if (host_card[1] < 4)AA_count++; /* �o�e�T�i�P */

        move(5, 0);  prints("�~�w�w�w��");
        move(6, 0);  prints("�x      �x");
        move(7, 0);  prints("�x      �x");
        move(8, 0);  prints("�x      �x");
        move(9, 0);  prints("�x      �x");
        move(10, 0); prints("�x      �x");
        move(11, 0); prints("���w�w�w��");
        print_card(host_card[1], 5, 4);
        print_card(guest_card[0], 15, 0);  /* �L�X�e�T�i�P */

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
                move(18, 3); prints("\x1b[1;41;33m     ������     \x1b[m");
                move(3, 0); prints("\x1b[1;41;33m������ !!! �o���� %d ������\x1b[m", money*seven);
                cuser_money += (money * seven);
//              inmoney(money*seven);
                game_log(2, "���F \x1b[1;33m%d\x1b[m ������ \x1b[1;31m  ������   \x1b[m"
                         , money*seven);
                pressanykey("�z�٦� \x1b[1;44;33m%d\x1b[m ����", cuser_money);
                flag = 1; m = 0;
            }

            if ((guest_card[0] == 40 && guest_card[1] == 0) || (guest_card[0] == 0 && guest_card[1] == 40))
            {
                move(18, 3); prints("\x1b[1;41;33m �W�ť��� BLACK JACK  \x1b[m");
                move(3, 0); prints("\x1b[1;41;33m�W�ť��� BLACK JACK !!! �o���� %d ����\x1b[m", money*super_jack);
                cuser_money += (money * super_jack);
                game_log(2, "���F \x1b[1;33m%d\x1b[m ������ \x1b[1;41;33m ���� �Ϣ� \x1b[m"
                         , money*super_jack);
                pressanykey("�z�٦� \x1b[1;44;33m%d\x1b[m ����", cuser_money);
                flag = 1; m = 0;
            }

            if ((guest_card[0] <= 3 && guest_card[0] >= 0) && (guest_card[1] <= 43 && guest_card[1] >= 40))tmp = 1;

            if ((tmp == 1) || ((guest_card[1] <= 3 && guest_card[1] >= 0) && (guest_card[0] <= 43 && guest_card[0] >= 40)))
            {
                move(18, 3); prints("\x1b[1;41;33m SUPER BLACK JACK  \x1b[m");
                move(3, 0); prints("\x1b[1;41;33mSUPER BLACK JACK !!! �o���� %d ����\x1b[m", money*aj);
                cuser_money += (money * aj);
//              inmoney(money*aj);
                game_log(2, "���F \x1b[1;33m%d\x1b[m ���� \x1b[1;44;33m Super�Ϣ� \x1b[m", money*aj);
                pressanykey("�z�٦� \x1b[1;44;33m%d\x1b[m ����", cuser_money);
                flag = 1; m = 0; tmp = 0;
            }

            if (guest_point == 21 && guest_count == 1)
            {
                move(18, 3); prints("\x1b[1;41;33m  BLACK JACK  \x1b[m");
                move(3, 0); prints("\x1b[1;41;33mBLACK JACK !!!\x1b[44m �o���� %d ����\x1b[m", money*win_jack);
                cuser_money += (money * win_jack);
//              inmoney(money*win_jack);
                move(0, 0); clrtoeol();//prints("�z�٦� \x1b[1;44;33m%d\x1b[m ����", cuser.money);
                if (money*win_jack >= 500000)
                {
                    game_log(2, "���F \x1b[1;33m%d\x1b[m ������ \x1b[1;47;30m BlackJack \x1b[m", money*win_jack);
                }

                pressanykey("�z�٦� \x1b[1;44;33m%d\x1b[m ����", cuser_money);
                flag = 1; m = 0;
            }                        /* �e��i�N 21 �I */

            if (guest_point > 21)
            {
                if (A_count > 0){guest_point -= 10; A_count--;}
            }
            move(12, 0); clrtoeol(); prints("\x1b[1;32m�I��: \x1b[33m%d\x1b[m", host_point);
            move(14, 0); clrtoeol(); prints("\x1b[1;32m�I��: \x1b[33m%d\x1b[m", guest_point);
            if (guest_point > 21)
            {
                pressanykey("  �z����~~~  ");
                flag = 1; m = 0;
            }

            if ((guest_count == 5) && (flag == 0))
            {
                move(18, 3); prints("\x1b[1;41;33m            �L����            \x1b[m");
                move(3, 0); prints("\x1b[1;41;33m�L���� !!! �o���� %d ����\x1b[m", money*six);
                cuser_money += (money * six);
//              inmoney(money*six);
//              inexp(ba*5);
                game_log(2, "���F \x1b[1;33m%d\x1b[m ���� \x1b[1;44;33m  �L����   \x1b[m", money*six);
                pressanykey("�z�٦� %d ����", cuser_money);
                flag = 1; m = 0;
//              return 0;
            }

            guest_count++;
            card_count++;
            mov_y += 4;

            do
            {
                if (ch == 'd')m = 0;
                if (m != 0) ch = vkey();
            }
            while (ch != 'y' && ch != 'n' && ch != 'd' && m != 0); /* �� key */

            if (ch == 'd' && m != 0 && guest_count == 2)
            {
                if (cuser_money >= money)
                {
                    cuser_money -= money;
//                  demoney(money);
                    money *= 2;
                }
                else ch = 'n';
                move(0, 0); clrtoeol(); prints("�z�٦� \x1b[1;44;33m%d\x1b[m ����", cuser_money);
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
                    if (AA_count > 0){host_point -= 10; AA_count--;}
                }
                move(12, 0); clrtoeol(); prints("\x1b[1;32m�I��: \x1b[33m%d\x1b[m", host_point);
                move(14, 0); clrtoeol(); prints("\x1b[1;32m�I��: \x1b[33m%d\x1b[m", guest_point);
                if (host_point > 21)
                {
                    move(14, 0); clrtoeol(); prints("\x1b[1;32m�I��: \x1b[33m%d \x1b[1;41;33m WINNER \x1b[m", guest_point);
                    cuser_money += (money * win);
//                  inmoney(money*win);
                    move(0, 0); clrtoeol(); prints("�z�٦� \x1b[1;44;33m%d\x1b[m ����", cuser_money);
                    pressanykey("\x1b[1;44;33m�AĹ�F~~~~ �o���� %d ����\x1b[m", money*win);
                    flag = 1;
                }
                host_count++;
                card_count++;
                mov_y += 4;
            }
            while (host_point < guest_point);
            if (!flag) pressanykey("�A��F~~~~ �����S��!");
        }
    }
}
