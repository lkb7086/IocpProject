#pragma once

const int MAX_DISCOMPORT_INDEX = 1;

enum class State : int
{
	NPC_NONE = 0,    // ����
	NPC_NORMAL = 1,	// �Ϲ�			
	NPC_DISCOMPORT = 2,	// ����
	NPC_ANGRY = 3,	// ȭ��

	NPC_STATE_CNT			// ���� ����
};

enum class Event : int
{
	EVENT_NONE = 0,			// ����
	EVENT_PLAYER_APPEAR = 1,  // �÷��̾ ����
	EVENT_PLAYER_ATTACK = 2,  // �÷��̾ ������
	EVENT_PLAYER_RUNAWAY = 3,  // �÷��̾ ������
	EVENT_DISCOMPORT_INDEX = 4, // �÷��̾ ��� �����ϰ���
};

static char strtableState[][20] =
{
	"NPC_NONE",
	"NPC_NORMAL",
	"NPC_DISCOMPORT",
	"NPC_ANGRY"
};

class FSM final
{
public:
	FSM() = delete;
	~FSM() = delete;


	class FSMClass;

private:
	/////////////////////////////////////////////////////////////////////////
	// ���¿� �̺�Ʈ�� �����ϴ� ��ü
	class FSMState final
	{
		friend class FSMClass; // cFsmState������� cFsmClass������ ���� �� �� �ֵ���
	private:
		typedef pair< Event, State >  STATE_PAIR;
		typedef map< Event, State >	 STATE_MAP;
		typedef STATE_MAP::iterator	STATE_IT;


		State				m_eState;  // cFsmClass::InsertFsmState���� new�Ҷ� �ʱⰪ ������
		STATE_MAP			m_mapState;  // �̺�Ʈ�� ���¸� ����, �̺�Ʈ�� Ű�� �����ϰ� �׿� ���� ������ ���̵� ���¸� ����


		State	GetState() { return m_eState; }
		void	InsertState(Event inputEvent, State inputState);
		void	RemoveState(Event inputEvent);
		// ���̵� ���¸� ��ȯ
		State	GetNextState(Event inputEvent);
		DWORD	GetStateCount();

	private:
		FSMState(State inputState);
		~FSMState();
		//virtual ~cFsmState();
	};

public:
	/////////////////////////////////////////////////////////////////////////
	// ���°�ü�� ���¸� �����ϴ� ��ü
	class FSMClass final
	{
	private:
		typedef pair< State, FSMState* >  FSMSTATE_PAIR;
		typedef map< State, FSMState* >	 FSMSTATE_MAP;
		typedef FSMSTATE_MAP::iterator	FSMSTATE_IT;


		FSMState*					m_pCurFsmState;  // ���������� ���� �����Ǿ� �ִ� ���°�ü, new�� �ƴϰ� �ʿ��� ���°� ����Ű�⸸ ��
		FSMSTATE_MAP				m_mapFsmState;  // ������¿� cFsmState��ü�� ����, cFsmState* �������ǰ� ���̵� ���¸� ����
		Event						m_eCurEvent;  // ���� �̺�Ʈ


	public:
		// m_mapFsmState�� Ű�� ���� ����
		void	InsertFsmState(State inputState, Event inputEvent, State inputNextState);
		void	RemoveFsmState(State inputState, Event inputEvent);

		// ���� ���¸� ����
		void	SetCurFsmState(State inputState);
		// ������¸� ��ȯ
		State	GetCurState();
		State	GetNextState(Event inputEvent);
		// ������¿��� �ٸ����·� ���� ����
		void	TranslateState(Event inputEvent);
		inline	Event	GetCurEvent() { return m_eCurEvent; }

	public:
		FSMClass();
		~FSMClass();
		//virtual ~cFsmClass();
	};
};