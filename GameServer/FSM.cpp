#include "stdafx.h"
#include "FSM.h"

FSM::FSMState::FSMState(State enumState) : m_eState(enumState)
{
}

FSM::FSMState::~FSMState()
{
	m_mapState.clear();
}

void FSM::FSMState::InsertState(Event inputEvent, State inputState)
{
	STATE_IT state_it = m_mapState.find(inputEvent);
	if (state_it != m_mapState.end())
	{
		LOG(LOG_ERROR_NORMAL, "SYSTEM | cFsm::cFsmState::InsertState() | 이벤트(%d)는 m_mapState에 이미 존재하고 있다.", inputEvent);
		return;
	}
	m_mapState.insert(STATE_PAIR(inputEvent, inputState));
}

void FSM::FSMState::RemoveState(Event inputEvent)
{
	STATE_IT state_it = m_mapState.find(inputEvent);
	if (state_it == m_mapState.end())
	{
		LOG(LOG_ERROR_NORMAL,
			"SYSTEM | cFsm::cFsmState::RemoveState() | 이벤트(%d)는 m_mapState에 존재하지 않는다.",
			inputEvent);
		return;

	}
	m_mapState.erase(inputEvent);
}

State FSM::FSMState::GetNextState(Event inputEvent)
{
	STATE_IT state_it = m_mapState.find(inputEvent);
	if (state_it == m_mapState.end())
	{
		LOG(LOG_ERROR_NORMAL, "SYSTEM | cFsm::cFsmState::GetNextState() | 이벤트(%d)는 m_mapState에 존재하지 않는다.", inputEvent);
		return State::NPC_NONE;
	}
	return state_it->second;
}

DWORD FSM::FSMState::GetStateCount()
{
	return (DWORD)m_mapState.size();
}


/////////////////////////////////////////////////////////////////////////////
// cFSM::CFsmClass class
//
FSM::FSMClass::FSMClass()
	: m_pCurFsmState(0), m_eCurEvent(Event::EVENT_NONE)
{

}

FSM::FSMClass::~FSMClass()
{
	FSMSTATE_IT fsmstate_it;
	for (fsmstate_it = m_mapFsmState.begin(); fsmstate_it != m_mapFsmState.end(); fsmstate_it++)
	{
		FSMState* pFsmState = fsmstate_it->second;
		delete pFsmState;
	}
	m_mapFsmState.clear();
}


void FSM::FSMClass::InsertFsmState(State inputState, Event inputEvent, State inputNextState)
{
	FSMState* pFsmState = nullptr;
	FSMSTATE_IT fsmstate_it = m_mapFsmState.find(inputState);
	// 동일한 상태가 이미 존재 한다면 그 상태 객체를 가져와서 이벤트와 다음 상태를 추가
	if (fsmstate_it != m_mapFsmState.end())
	{
		pFsmState = (FSMState*)fsmstate_it->second;
		pFsmState->InsertState(inputEvent, inputNextState);
		return;
	}
	else // 동일한 상태가 없다면 상태 객체를 생성하고 이벤트와 다음 상태를 추가
	{
		pFsmState = new(nothrow)FSMState(inputState);
		m_mapFsmState.insert(FSMSTATE_PAIR(inputState, pFsmState));
	}
	pFsmState->InsertState(inputEvent, inputNextState);
}

void FSM::FSMClass::RemoveFsmState(State inputState, Event inputEvent)
{
	FSMSTATE_IT fsmstate_it = m_mapFsmState.find(inputState);
	//해당 상태객체가 존재하지 않는다면
	if (fsmstate_it == m_mapFsmState.end())
	{
		LOG(LOG_ERROR_NORMAL,
			"SYSTEM | cFsm::cFsmClass::RemoveFsmState | 상태(%d) 객체는 m_mapFsmState에 존재하지 않는다.",
			inputState);
		return;
	}
	FSMState* pFsmState = fsmstate_it->second;
	//이벤트에 해당하는 상태 정보를 지운다
	pFsmState->RemoveState(inputEvent);
	//상태 객체에 상태 정보가 없다면 객체를 지운다.
	if (pFsmState->GetStateCount() == 0)
	{
		delete pFsmState;
		m_mapFsmState.erase(fsmstate_it);
	}
}

void FSM::FSMClass::SetCurFsmState(State inputState)
{
	FSMSTATE_IT fsmstate_it = m_mapFsmState.find(inputState);
	//해당 상태객체가 존재하지 않는다면
	if (fsmstate_it == m_mapFsmState.end())
	{
		LOG(LOG_ERROR_NORMAL, "SYSTEM | cFsm::cFsmClass::SetCurFsmState | 상태(%d) 객체는 m_mapFsmState에 존재하지 않는다.", inputState);
		return;
	}
	m_pCurFsmState = fsmstate_it->second;
}

State FSM::FSMClass::GetCurState()
{
	//현재 상태가 설정되어 있다면
	if (m_pCurFsmState)
		return m_pCurFsmState->GetState();
	return State::NPC_NONE;
}

State FSM::FSMClass::GetNextState(Event inputEvent)
{
	//현재 상태가 설정되어 있다면
	if (m_pCurFsmState)
	{
		m_pCurFsmState->GetNextState(inputEvent);
		return m_pCurFsmState->GetState();
	}
	return State::NPC_NONE;
}

void FSM::FSMClass::TranslateState(Event inputEvent)
{
	if (m_pCurFsmState)
	{
		State enumCurState = m_pCurFsmState->GetState();
		State enumNextState = m_pCurFsmState->GetNextState(inputEvent);  // 플레이어등장 이벤트면 -> enumNextState은 불쾌

		FSMSTATE_IT fsmstate_it = m_mapFsmState.find(enumNextState);
		//해당 상태객체가 존재하지 않는다면
		if (fsmstate_it == m_mapFsmState.end())
		{
			LOG(LOG_ERROR_LOW, "SYSTEM | cFsm::cFsmClass::TranslateState | 상태(%d) 객체는 m_mapFsmState에 존재하지 않는다.", enumNextState);
			return;
		}
		m_pCurFsmState = fsmstate_it->second;   // cFsmState* 할당
		m_eCurEvent = inputEvent;
		//LOG( LOG_INFO_LOW, "SYSTEM | cFsm::cFsmClass::TranslateState() | NPC상태 변경 %s -> %s", strtableState[ enumCurState ] , strtableState[ enumNextState ] );
	}
}
