#pragma once
#include "NPC.h"

class DetectNPC final : public CNPC
{
public:
	explicit DetectNPC();
	~DetectNPC();

	virtual void	OnProcess() override;
	virtual void	Init() override;
};

