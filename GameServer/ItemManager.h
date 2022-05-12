#pragma once

struct Item
{
	unsigned short code;
	bool flag;
	float x; float y; float z;
	Item() { memset(this, 0, sizeof(Item)); }
};

class CItemManager final : public CSingleton
{
	DECLEAR_SINGLETON(CItemManager);
public:
	CItemManager();
	~CItemManager();

	inline unsigned short GenerateItemKey() { return ++m_generateItemKey; }
	void InsertItem(unsigned short itemKey, const Item& item)
	{
		auto it = m_mapItem.find(itemKey);
		if (it == m_mapItem.end())
			m_mapItem.insert(pair< unsigned __int32, Item >(itemKey, item));
		else
			LOG(LOG_ERROR_NORMAL, "SYSTEM | CItemManager::Insert_MapItemKey() | 이미 추가하려는 아이템키가 있다");
	}

	bool EraseItem(unsigned short itemKey)
	{
		auto it = m_mapItem.find(itemKey);
		if (it != m_mapItem.end())
		{
			m_mapItem.erase(itemKey);
			return true;
		}
		else
		{
			LOG(LOG_ERROR_NORMAL, "SYSTEM | CItemManager::Erase_MapItemKey() | 삭제하려는 아이템키가 없다");
			return false;
		}
	}

	inline void Clear_MapItem()
	{
		if (!m_mapItem.empty())
			m_mapItem.clear();
	}

	Item* FindItem(unsigned short itemKey)
	{
		auto it = m_mapItem.find(itemKey);
		if (it != m_mapItem.end())
		{
			return (Item*)&(it->second);
		}
		else
		{
			//LOG(LOG_ERROR_NORMAL, "SYSTEM | CItemManager::Find_MapItem() | 아이템키가 없다");
			return nullptr;
		}
	}

	void InitItem();

	void GS_CL_InitItemInfo(CPlayer* pPlayer);
	
	void Recv_GetItem(CPlayer* pPlayer, char* pRecvedMsg)
	{
		unsigned short itemKey = 0;
		tls_pSer->StartDeserialize(pRecvedMsg);
		tls_pSer->Deserialize(itemKey);

		Item* pItem = FindItem(itemKey);
		if (pItem == nullptr)
			return;
		// 해당 아이템에 맞게 플레이어게 조치를 하고
		// 컨테이너에서 아이템 삭제하고
		EraseItem(itemKey);

		// 먹었다고 보낸다
		PlayerManager()->GS_CL_GetItem(pPlayer, itemKey);
	}

	void CL_GS_GetVictim(char* pMsg)
	{
		unsigned short code = 0;
		Vector3 pos;
		CSerializer& ser = *tls_pSer;
		ser.StartDeserialize(pMsg);
		ser.Deserialize(code);
		ser.Deserialize(pos.x);
		ser.Deserialize(pos.y);
		ser.Deserialize(pos.z);

		for (auto it = m_listVictim.begin(); it != m_listVictim.end(); ++it)
		{
			if (it->code == code)
			{
				it->x = pos.x; it->y = pos.y; it->z = pos.z;
				PlayerManager()->GS_CL_GetVictim(code);
				break;
			}
		}
	}

private:
	unsigned short m_generateItemKey;
	std::list<Item> m_listVictim;

	unordered_map< unsigned short, Item > m_mapItem;
};
CREATE_FUNCTION(CItemManager, ItemManager);