#pragma once

// ������: native(�����ڸ�) -> ��������Ʈ
// detect�� ������ �÷��̾ �ƴ� npc�� �д�

class CMovingNPC final : public CNPC
{
public:
	explicit CMovingNPC();
	~CMovingNPC();

	virtual void	OnProcess() override;
	virtual void	Init() override;
};