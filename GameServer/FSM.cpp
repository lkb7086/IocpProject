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
		LOG(LOG_ERROR_NORMAL, "SYSTEM | cFsm::cFsmState::InsertState() | �̺�Ʈ(%d)�� m_mapState�� �̹� �����ϰ� �ִ�.", inputEvent);
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
			"SYSTEM | cFsm::cFsmState::RemoveState() | �̺�Ʈ(%d)�� m_mapState�� �������� �ʴ´�.",
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
		LOG(LOG_ERROR_NORMAL, "SYSTEM | cFsm::cFsmState::GetNextState() | �̺�Ʈ(%d)�� m_mapState�� �������� �ʴ´�.", inputEvent);
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
	// ������ ���°� �̹� ���� �Ѵٸ� �� ���� ��ü�� �����ͼ� �̺�Ʈ�� ���� ���¸� �߰�
	if (fsmstate_it != m_mapFsmState.end())
	{
		pFsmState = (FSMState*)fsmstate_it->second;
		pFsmState->InsertState(inputEvent, inputNextState);
		return;
	}
	else // ������ ���°� ���ٸ� ���� ��ü�� �����ϰ� �̺�Ʈ�� ���� ���¸� �߰�
	{
		pFsmState = new(nothrow)FSMState(inputState);
		m_mapFsmState.insert(FSMSTATE_PAIR(inputState, pFsmState));
	}
	pFsmState->InsertState(inputEvent, inputNextState);
}

void FSM::FSMClass::RemoveFsmState(State inputState, Event inputEvent)
{
	FSMSTATE_IT fsmstate_it = m_mapFsmState.find(inputState);
	//�ش� ���°�ü�� �������� �ʴ´ٸ�
	if (fsmstate_it == m_mapFsmState.end())
	{
		LOG(LOG_ERROR_NORMAL,
			"SYSTEM | cFsm::cFsmClass::RemoveFsmState | ����(%d) ��ü�� m_mapFsmState�� �������� �ʴ´�.",
			inputState);
		return;
	}
	FSMState* pFsmState = fsmstate_it->second;
	//�̺�Ʈ�� �ش��ϴ� ���� ������ �����
	pFsmState->RemoveState(inputEvent);
	//���� ��ü�� ���� ������ ���ٸ� ��ü�� �����.
	if (pFsmState->GetStateCount() == 0)
	{
		delete pFsmState;
		m_mapFsmState.erase(fsmstate_it);
	}
}

void FSM::FSMClass::SetCurFsmState(State inputState)
{
	FSMSTATE_IT fsmstate_it = m_mapFsmState.find(inputState);
	//�ش� ���°�ü�� �������� �ʴ´ٸ�
	if (fsmstate_it == m_mapFsmState.end())
	{
		LOG(LOG_ERROR_NORMAL, "SYSTEM | cFsm::cFsmClass::SetCurFsmState | ����(%d) ��ü�� m_mapFsmState�� �������� �ʴ´�.", inputState);
		return;
	}
	m_pCurFsmState = fsmstate_it->second;
}

State FSM::FSMClass::GetCurState()
{
	//���� ���°� �����Ǿ� �ִٸ�
	if (m_pCurFsmState)
		return m_pCurFsmState->GetState();
	return State::NPC_NONE;
}

State FSM::FSMClass::GetNextState(Event inputEvent)
{
	//���� ���°� �����Ǿ� �ִٸ�
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
		State enumNextState = m_pCurFsmState->GetNextState(inputEvent);  // �÷��̾���� �̺�Ʈ�� -> enumNextState�� ����

		FSMSTATE_IT fsmstate_it = m_mapFsmState.find(enumNextState);
		//�ش� ���°�ü�� �������� �ʴ´ٸ�
		if (fsmstate_it == m_mapFsmState.end())
		{
			LOG(LOG_ERROR_LOW, "SYSTEM | cFsm::cFsmClass::TranslateState | ����(%d) ��ü�� m_mapFsmState�� �������� �ʴ´�.", enumNextState);
			return;
		}
		m_pCurFsmState = fsmstate_it->second;   // cFsmState* �Ҵ�
		m_eCurEvent = inputEvent;
		//LOG( LOG_INFO_LOW, "SYSTEM | cFsm::cFsmClass::TranslateState() | NPC���� ���� %s -> %s", strtableState[ enumCurState ] , strtableState[ enumNextState ] );
	}
}
