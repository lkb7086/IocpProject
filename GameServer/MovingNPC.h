#pragma once

// 움직임: native(스폰자리) -> 웨이포인트
// detect와 공격을 플레이어가 아닌 npc로 둔다

class CMovingNPC final : public CNPC
{
public:
	explicit CMovingNPC();
	~CMovingNPC();

	virtual void	OnProcess() override;
	virtual void	Init() override;
};