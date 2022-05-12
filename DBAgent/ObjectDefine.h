typedef struct item
{
	char				is_init;
	char				idx_type; 	//�ε���Ÿ��
	unsigned short		item_no;	//������ ��ȣ�� ������ �迭�� �ε��� (������ ������ȣ)..
	short				req;		//�䱸����
	char				equip;		//������ġ 0:�Ӹ� 1:�Ѽ�(������) 2:��� 3:���� 4:�� 5:����� 6:����
	char				item_type;	//��� Ÿ��
	unsigned short		pwr;		//���ݷ� �Ǵ� ����
	char				range;		//Ÿ�ݰŸ�
	char				itemclass;	//���迭 0:���� 1:���� 2:��Ÿ
	unsigned short		amount;		//�⺻����
	unsigned short		coin;	    //����
	char				isbuy;		//���԰��ɿ���
}type_item;

typedef struct npc_default
{
	unsigned short no;
	unsigned short gtype;
	unsigned short level;
	unsigned short speed;

	unsigned short item[12];
	unsigned short rate[12];
}type_gnpc;

typedef struct skills{
	unsigned char 	type;   		// Ÿ�� 0: ���� �迭 ���� 1: �ߵ� �迭 ���� 2: ���� ��ȭ �迭
	unsigned char 	skillidx;   	// ��ų ��ȣ

	unsigned char 	lvpwr[11];   	// ������ ���� ���ݷ�

	unsigned int 	lvupcost[11];	// ��ų �������� ��� ��
	unsigned int	getcoin;	    // ��ų ��� �� ��� ��
}skill_def;                      	// idx 0�� ����
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////









typedef struct partySystem
{
	unsigned char	flag;		// ��Ƽ ���� ����
	unsigned char	cnt;		// ��Ƽ �ο���

	short		memidx;		// �ڽ��� ��ġ
	unsigned short	infoFcnt;	// ���� ���� �˸� ī��Ʈ

	void *		waitfor;	// ��ٸ��� �ִ� ���� ��ȣ

	void *		master;
	short		ison1;		// 0: NULL	1:linked	2:myself(member#==NULL)

	void *		mem2;
	short		ison2;

	void *		mem3;
	short		ison3;

}type_party;

typedef struct tradeSystem
{
	short		flag;			// �ŷ� �� ����
	void *		tg;
	short		confirm;

	unsigned char		itemidx[2];	// �ŷ� �κ��丮 ���� ���� ���� �κ��丮 �ε���
	unsigned short		invidx[2];	// ������ �ε���
	unsigned short		cnt[2];		// ������ ī��Ʈ
	unsigned int		coin;
}type_trade;  // �ŷ��� ���� ����ü

typedef struct connection
{
	char		userid[32];
	char		char_name[32];
	unsigned short  uLen;
	unsigned short  nLen;
	unsigned short  userNo;
	int             sock;
	unsigned short  port;
	unsigned int    addr;
	//type_conn_state state;
	unsigned int    sessionkey;
	unsigned int 	clientver;
	int		disco;

	unsigned char	dbLoaded;	// DB �ε� ����
	short		frameCnt;	// ������ ī��Ʈ
	short		userStat;	// ���� ���� 0: ���� 1: �̵� 2: ���� 3: �ŷ�
	short		roomNo;		// �⺻: 0


	/****ĳ���� ����****/
	unsigned short	Cx;	//current loc x ������ġ
	unsigned short	Cy;	//current loc y
	unsigned short	Dx;	//destination x
	unsigned short	Dy;	//destination y

	short		Ax;	//current sector X
	short		Ay;	//current sector Y
	short		Bx;	//before  sector X
	short		By;	//before  sector Y

	char		race;
	char		sex;
	char		nation;

	unsigned short	str;		// ��
	unsigned short	dex;		// ��ø
	unsigned short	intel;		// ����

	unsigned short	attack;		// ���ݷ�
	unsigned short	defence;	// ����

	unsigned short	hp_m;		// �ִ� ü��
	short		hp_c;		// ���� ü��
	short		mana_m;		// �ִ� ����
	short		mana_c;		// ���� ����

	unsigned int	exp;		// ����ġ
	unsigned short	level;		// ����
	unsigned short	lvpoint;	// ���� ����Ʈ
	unsigned int	skexp;		// ��ų ����ġ

	char		jobno;		// ���� ��ȣ
	char		classno;	// Ŭ���� ��ȣ
	char		classlevel;	// Ŭ���� ����

	unsigned short 	worldmap;	// �� �ε���
	unsigned short	moveLevel; 	// ĳ������ ������ ����
	char		Dir;		// ������ ����

	/*****Ÿ�� ���ϱ� ********/
	char		tgtype;		// Ÿ�� Ÿ��
	short 		target_id;	// Ÿ�� ���̵�
	void *		target;		// Ÿ�� ����ü

	/****������ ����****/
	unsigned short	eq[7];		// ��� �迭

	/****�κ��丮 ����****/
	unsigned short	inven[4];	// �κ��丮 4��
	unsigned char	inven_cnt[4];	// �κ��丮 48��

	/**** â�� ����****/
	unsigned short	ware[8];	// â�� 8��
	unsigned char	ware_cnt[8];	// â�� 8��

	/****��ų ����****/
	unsigned short	skill[4];
	unsigned char	skill_lv[4];

	unsigned int 	coin;		// ��

	char		move_F_cnt;
	unsigned short	mana_F_cnt;	// ĳ���� ���� ȸ���� ī��Ʈ
	unsigned short	recov_F_cnt;	// ĳ���� ü�� ȸ���� ī��Ʈ
	short		attack_F_cnt;	// ĳ���� ���� ������ ī��Ʈ
	short		regen_F_cnt;
	unsigned int	dbSaveCnt;

	unsigned char	pkmode;
	type_party	party;
	type_trade	trade;
}
type_session;	//���� ����

typedef struct connArray
{
	type_session	s;
	int		sockNo;
	char		flag;
}
t_sessionArray;	// ��ü ������ ���� �迭