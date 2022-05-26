#pragma once



class CItemManager final : public CSingleton
{
	DECLEAR_SINGLETON(CItemManager);
public:
	CItemManager();
	~CItemManager();


};
CREATE_FUNCTION(CItemManager, ItemManager);