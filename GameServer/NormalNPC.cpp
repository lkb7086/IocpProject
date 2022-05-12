#include "stdafx.h"
#include "NormalNPC.h"

NormalNPC::NormalNPC() : m_smallStartWaveFlag(false)
{
	Init();
}

NormalNPC::~NormalNPC()
{
}

void NormalNPC::Init()
{
	m_pFsmClass->InsertFsmState(State::NPC_NORMAL, Event::EVENT_PLAYER_APPEAR, State::NPC_ANGRY);
	m_pFsmClass->InsertFsmState(State::NPC_NORMAL, Event::EVENT_PLAYER_ATTACK, State::NPC_ANGRY);
	//m_pFsmClass->InsertFsmState( NPC_DISCOMPORT , EVENT_PLAYER_ATTACK , NPC_ANGRY );
	//m_pFsmClass->InsertFsmState( NPC_DISCOMPORT , EVENT_DISCOMPORT_INDEX , NPC_ANGRY );
	//m_pFsmClass->InsertFsmState( NPC_DISCOMPORT , EVENT_PLAYER_RUNAWAY , NPC_NORMAL );
	m_pFsmClass->InsertFsmState(State::NPC_ANGRY, Event::EVENT_PLAYER_RUNAWAY, State::NPC_NORMAL);

	// 디폴트 설정
	m_pFsmClass->SetCurFsmState(State::NPC_NORMAL);

	InitDetect();
}

void NormalNPC::OnProcess()
{
	if (false == m_smallStartWaveFlag)
		return;
	if (CNPC::CheckDead())
		return;

	switch (m_pFsmClass->GetCurState())
	{
	case State::NPC_NORMAL:
	{
		PlayerManager()->DoTargetWorldPlayer(this);
		//DoWander();
	}
	break;
	default:
		break;
	}

	CNPC::OnProcess();
}