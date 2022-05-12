#pragma once
#include "NPC.h"

class NormalNPC final : public CNPC
{
public:
	explicit NormalNPC();
	~NormalNPC();
	virtual void	OnProcess() override;
	virtual void	Init() override;

	bool m_smallStartWaveFlag;
private:
	//bool smallStartWaveFlag;
};

