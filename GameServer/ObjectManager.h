#pragma once




struct Cube
{
	unsigned int uid;
	float r; float g; float b; float a;

	Cube()
	{
		memset(this, 0, sizeof(Cube));
		r = 0.5f; g = 0.5f; b = 0.5f; a = 1.0f;
	}
};



class CObjectManager final : public CSingleton
{
	DECLEAR_SINGLETON(CObjectManager);

public:
	CObjectManager();





	Cube* FindCube(unsigned int key)
	{
		auto it = m_mapCube.find(key);
		if (it != m_mapCube.end())
		{
			return it->second;
		}
		else
			return nullptr;
	}


	void CreateObjects_Not(CPlayer* pPlayer)
	{
		tls_pSer->StartSerialize();
		tls_pSer->Serialize(static_cast<packet_type>(PacketType::CreateObjects_Not));
		tls_pSer->Serialize((unsigned short)10);
		for (unsigned int i = 0; i < 10; i++)
		{
			tls_pSer->Serialize(m_pCube[i].uid);
			tls_pSer->Serialize(m_pCube[i].r);
			tls_pSer->Serialize(m_pCube[i].g);
			tls_pSer->Serialize(m_pCube[i].b);
			tls_pSer->Serialize(m_pCube[i].a);

		}

		char* pSendBuffer = pPlayer->PrepareSendPacket(tls_pSer->GetCurBufSize());
		if (nullptr == pSendBuffer)
			return;
		tls_pSer->CopyBuffer(pSendBuffer);
		pPlayer->SendPost(tls_pSer->GetCurBufSize());
	}



private:
	map<unsigned int, Cube*> m_mapCube;

	Cube* m_pCube;
};

CREATE_FUNCTION(CObjectManager, ObjectManager);

