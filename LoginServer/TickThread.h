#pragma once

class CTickThread final : public CThread
{
public:
	CTickThread();
	~CTickThread();

	virtual void OnInitThread() override;
	virtual void OnCloseThread() override;
	virtual void OnProcess() override;
};