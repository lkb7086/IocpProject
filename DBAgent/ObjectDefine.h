typedef struct item
{
	char				is_init;
	char				idx_type; 	//인덱스타입
	unsigned short		item_no;	//아이템 번호는 아이템 배열의 인덱스 (아이템 고유번호)..
	short				req;		//요구레벨
	char				equip;		//장착위치 0:머리 1:한손(오른손) 2:양손 3:몸통 4:발 5:목걸이 6:반지
	char				item_type;	//장비 타입
	unsigned short		pwr;		//공격력 또는 방어력
	char				range;		//타격거리
	char				itemclass;	//사용계열 0:무기 1:갑옷 2:기타
	unsigned short		amount;		//기본개수
	unsigned short		coin;	    //가격
	char				isbuy;		//구입가능여부
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
	unsigned char 	type;   		// 타입 0: 물리 계열 공격 1: 중독 계열 공격 2: 상태 변화 계열
	unsigned char 	skillidx;   	// 스킬 번호

	unsigned char 	lvpwr[11];   	// 레벨에 따른 공격력

	unsigned int 	lvupcost[11];	// 스킬 레벨업에 드는 돈
	unsigned int	getcoin;	    // 스킬 배울 때 드는 돈
}skill_def;                      	// idx 0은 없음
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////









typedef struct partySystem
{
	unsigned char	flag;		// 파티 참석 여부
	unsigned char	cnt;		// 파티 인원수

	short		memidx;		// 자신의 위치
	unsigned short	infoFcnt;	// 유저 정보 알림 카운트

	void *		waitfor;	// 기다리고 있는 유저 번호

	void *		master;
	short		ison1;		// 0: NULL	1:linked	2:myself(member#==NULL)

	void *		mem2;
	short		ison2;

	void *		mem3;
	short		ison3;

}type_party;

typedef struct tradeSystem
{
	short		flag;			// 거래 중 여부
	void *		tg;
	short		confirm;

	unsigned char		itemidx[2];	// 거래 인벤토리 실제 저장 값은 인벤토리 인덱스
	unsigned short		invidx[2];	// 아이템 인덱스
	unsigned short		cnt[2];		// 아이템 카운트
	unsigned int		coin;
}type_trade;  // 거래를 위한 구조체

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

	unsigned char	dbLoaded;	// DB 로드 유무
	short		frameCnt;	// 프레임 카운트
	short		userStat;	// 유저 상태 0: 멈춤 1: 이동 2: 전투 3: 거래
	short		roomNo;		// 기본: 0


	/****캐릭터 관련****/
	unsigned short	Cx;	//current loc x 현재위치
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

	unsigned short	str;		// 힘
	unsigned short	dex;		// 민첩
	unsigned short	intel;		// 지식

	unsigned short	attack;		// 공격력
	unsigned short	defence;	// 방어력

	unsigned short	hp_m;		// 최대 체력
	short		hp_c;		// 현재 체력
	short		mana_m;		// 최대 마나
	short		mana_c;		// 현재 마나

	unsigned int	exp;		// 경험치
	unsigned short	level;		// 레벨
	unsigned short	lvpoint;	// 레벨 포인트
	unsigned int	skexp;		// 스킬 경험치

	char		jobno;		// 직업 번호
	char		classno;	// 클래스 번호
	char		classlevel;	// 클래스 레벨

	unsigned short 	worldmap;	// 맵 인덱스
	unsigned short	moveLevel; 	// 캐릭터의 움직임 레벨
	char		Dir;		// 목적지 방향

	/*****타겟 정하기 ********/
	char		tgtype;		// 타겟 타입
	short 		target_id;	// 타겟 아이디
	void *		target;		// 타겟 구조체

	/****아이템 정의****/
	unsigned short	eq[7];		// 장비 배열

	/****인벤토리 정의****/
	unsigned short	inven[4];	// 인벤토리 4개
	unsigned char	inven_cnt[4];	// 인벤토리 48개

	/**** 창고 정의****/
	unsigned short	ware[8];	// 창고 8개
	unsigned char	ware_cnt[8];	// 창고 8개

	/****스킬 정의****/
	unsigned short	skill[4];
	unsigned char	skill_lv[4];

	unsigned int 	coin;		// 돈

	char		move_F_cnt;
	unsigned short	mana_F_cnt;	// 캐릭터 마나 회복용 카운트
	unsigned short	recov_F_cnt;	// 캐릭터 체력 회복용 카운트
	short		attack_F_cnt;	// 캐릭터 공격 지연용 카운트
	short		regen_F_cnt;
	unsigned int	dbSaveCnt;

	unsigned char	pkmode;
	type_party	party;
	type_trade	trade;
}
type_session;	//세션 정보

typedef struct connArray
{
	type_session	s;
	int		sockNo;
	char		flag;
}
t_sessionArray;	// 전체 접속자 접속 배열