#pragma once

const int MAX_DISCOMPORT_INDEX = 1;

enum class State : int
{
	NPC_NONE = 0,    // 없음
	NPC_NORMAL = 1,	// 일반			
	NPC_DISCOMPORT = 2,	// 불쾌
	NPC_ANGRY = 3,	// 화남

	NPC_STATE_CNT			// 상태 개수
};

enum class Event : int
{
	EVENT_NONE = 0,			// 없음
	EVENT_PLAYER_APPEAR = 1,  // 플레이어가 등장
	EVENT_PLAYER_ATTACK = 2,  // 플레이어가 공격함
	EVENT_PLAYER_RUNAWAY = 3,  // 플레이어가 도망감
	EVENT_DISCOMPORT_INDEX = 4, // 플레이어가 계속 불쾌하게함
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
	// 상태와 이벤트를 관리하는 객체
	class FSMState final
	{
		friend class FSMClass; // cFsmState멤버들을 cFsmClass에서만 접근 할 수 있도록
	private:
		typedef pair< Event, State >  STATE_PAIR;
		typedef map< Event, State >	 STATE_MAP;
		typedef STATE_MAP::iterator	STATE_IT;


		State				m_eState;  // cFsmClass::InsertFsmState에서 new할때 초기값 설정됨
		STATE_MAP			m_mapState;  // 이벤트와 상태를 관리, 이벤트를 키로 설정하고 그에 대한 값으로 전이된 상태를 설정


		State	GetState() { return m_eState; }
		void	InsertState(Event inputEvent, State inputState);
		void	RemoveState(Event inputEvent);
		// 전이된 상태를 반환
		State	GetNextState(Event inputEvent);
		DWORD	GetStateCount();

	private:
		FSMState(State inputState);
		~FSMState();
		//virtual ~cFsmState();
	};

public:
	/////////////////////////////////////////////////////////////////////////
	// 상태객체와 상태를 관리하는 객체
	class FSMClass final
	{
	private:
		typedef pair< State, FSMState* >  FSMSTATE_PAIR;
		typedef map< State, FSMState* >	 FSMSTATE_MAP;
		typedef FSMSTATE_MAP::iterator	FSMSTATE_IT;


		FSMState*					m_pCurFsmState;  // 최종적으로 현재 설정되어 있는 상태객체, new가 아니고 맵에서 빼온거 가리키기만 함
		FSMSTATE_MAP				m_mapFsmState;  // 현재상태와 cFsmState객체를 관리, cFsmState* 전이조건과 전이된 상태를 가짐
		Event						m_eCurEvent;  // 현재 이벤트


	public:
		// m_mapFsmState에 키와 값을 설정
		void	InsertFsmState(State inputState, Event inputEvent, State inputNextState);
		void	RemoveFsmState(State inputState, Event inputEvent);

		// 현재 상태를 설정
		void	SetCurFsmState(State inputState);
		// 현재상태를 반환
		State	GetCurState();
		State	GetNextState(Event inputEvent);
		// 현재상태에서 다른상태로 상태 전이
		void	TranslateState(Event inputEvent);
		inline	Event	GetCurEvent() { return m_eCurEvent; }

	public:
		FSMClass();
		~FSMClass();
		//virtual ~cFsmClass();
	};
};