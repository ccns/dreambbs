
// �ɯŻݪᤧ exp
#define LVUP (rpguser.level*rpguser.level)*10000

// �U¾�~�ɯżW�[���ݩ�
int lvup[5][7]={{0,3,2,1,3,2,2},
                {0,2,3,2,1,1,2},
                {0,2,2,2,3,3,2},
                {0,2,2,3,1,2,1},
                {0,1,1,2,2,2,3}};

// �V�m��������
enum
{easy,medium,hard}; 

// mob�ݩ�
struct mobattr
{
  char name[20]; int LV; int maxhp; int hp; int Att;
  int Def; int MA; int MD; int EXP; int money;
};
typedef struct mobattr mobattr;

// ¾�~�W��
char *rname[7] = 
  {RACE_NORACE,RACE_POST,RACE_READ,RACE_IDLE,RACE_CHAT,RACE_MSG,RACE_GAME};

// ����������
char *body[5] = {"�Y��","����","�U��","�y��","�L��"};

// ���骺���A
char *health[5]={"[1;36m�@�v�L��[m","[1;32m�y������[m","[1;33m���I�~�F[m","[1;35m�����ˤf[m","[1;31m�a�a�@��[m"};
