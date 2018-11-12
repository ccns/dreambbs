
// 升級需花之 exp
#define LVUP (rpguser.level*rpguser.level)*10000

// 各職業升級增加之屬性
int lvup[5][7]={{0,3,2,1,3,2,2},
                {0,2,3,2,1,1,2},
                {0,2,2,2,3,3,2},
                {0,2,2,3,1,2,1},
                {0,1,1,2,2,2,3}};

// 訓練場的難度
enum
{easy,medium,hard}; 

// mob屬性
struct mobattr
{
  char name[20]; int LV; int maxhp; int hp; int Att;
  int Def; int MA; int MD; int EXP; int money;
};
typedef struct mobattr mobattr;

// 職業名稱
char *rname[7] = 
  {RACE_NORACE,RACE_POST,RACE_READ,RACE_IDLE,RACE_CHAT,RACE_MSG,RACE_GAME};

// 攻擊的部位
char *body[5] = {"頭部","身體","下面","腰部","腿部"};

// 身體的狀態
char *health[5]={"\x1b[1;36m毫髮無傷\x1b[m","\x1b[1;32m稍有受傷\x1b[m","\x1b[1;33m有點腫了\x1b[m","\x1b[1;35m滿身傷口\x1b[m","\x1b[1;31m奄奄一息\x1b[m"};
