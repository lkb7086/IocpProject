#include "stdafx.h"
#include "DetectNPC.h"

DetectNPC::DetectNPC()
{
	Init();
}

DetectNPC::~DetectNPC()
{
}

void DetectNPC::Init()
{
	// 1key, 2key(1key의 데이터), 다수
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

void DetectNPC::OnProcess()
{
	if (CNPC::CheckDead())
		return;

	switch (m_pFsmClass->GetCurState())
	{
	case State::NPC_NORMAL:
	{
		DoDetect();
		//DoWander();
	}
	break;
	default:
		break;
	}

	CNPC::OnProcess();
}