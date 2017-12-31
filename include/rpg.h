
// ¤É¯Å»Ýªá¤§ exp
#define LVUP (rpguser.level*rpguser.level)*10000

// ¦UÂ¾·~¤É¯Å¼W¥[¤§ÄÝ©Ê
int lvup[5][7]={{0,3,2,1,3,2,2},
                {0,2,3,2,1,1,2},
                {0,2,2,2,3,3,2},
                {0,2,2,3,1,2,1},
                {0,1,1,2,2,2,3}};

// °V½m³õªºÃø«×
enum
{easy,medium,hard}; 

// mobÄÝ©Ê
struct mobattr
{
  char name[20]; int LV; int maxhp; int hp; int Att;
  int Def; int MA; int MD; int EXP; int money;
};
typedef struct mobattr mobattr;

// Â¾·~¦WºÙ
char *rname[7] = 
  {RACE_NORACE,RACE_POST,RACE_READ,RACE_IDLE,RACE_CHAT,RACE_MSG,RACE_GAME};

// §ðÀ»ªº³¡¦ì
char *body[5] = {"ÀY³¡","¨­Åé","¤U­±","¸y³¡","»L³¡"};

// ¨­Åéªºª¬ºA
char *health[5]={"[1;36m²@¾vµL¶Ë[m","[1;32mµy¦³¨ü¶Ë[m","[1;33m¦³ÂI¸~¤F[m","[1;35mº¡¨­¶Ë¤f[m","[1;31m©a©a¤@®§[m"};
