#include "pch.h"
#include "CMonsterMgr.h"

IMPLEMENT_SINGLETON(CMonsterMgr)

CMonsterMgr::CMonsterMgr()
{
}

CMonsterMgr::~CMonsterMgr()
{
}

HRESULT CMonsterMgr::Ready_MonsterMgr()
{
	return E_NOTIMPL;
}

void CMonsterMgr::Update(const _float& fTimeDelta)
{
	//triggerID가 0인 몬스터들은 그냥 스폰
	

	//triggerID가 0이 아닌 몬스터들은 나중에 스폰


	for (auto& pair : m_mapMonsterGroups)
	{
		//활성화된 TriggerID의 오브젝트들만 
		if (pair.first == m_iTriggerID)
		{
			for (auto& pMonster : pair.second)
			{
				pMonster->Update_GameObject(fTimeDelta);
			}
		}
	}
}

void CMonsterMgr::LateUpdate(const _float & fTimeDelta)
{
	for (auto& pair : m_mapMonsterGroups)
	{
		if (pair.first == m_iTriggerID)
		{
			for (auto& pMonster : pair.second)
			{
				pMonster->LateUpdate_GameObject(fTimeDelta);
			}
		}
	}
}

void CMonsterMgr::Render()
{
	for (auto& pair : m_mapMonsterGroups)
	{
		if (pair.first == m_iTriggerID)
		{
			for (auto& pMonster : pair.second)
			{
				pMonster->Render_GameObject();
			}
		}
	}
}

void CMonsterMgr::SpawnMonsters(const _float& fTimeDelta)
{
	m_iSpawnDelay += fTimeDelta;
	
	for (auto& pair : m_mapMonsterGroups)
	{
		if (pair.first == m_iTriggerID)
		{
			if (m_iSpawnDelay <= 5.f)
				break;
			for (auto& pMonster : pair.second)
			{
				
			}
		}
	}
}

bool CMonsterMgr::IsGroupAllDead(int iTriggerID)
{
	auto iter = m_mapMonsterGroups.find(iTriggerID);
	if (iter == m_mapMonsterGroups.end())
		return false;
	//몬스터 Alive 플래그 사용해서 판단
}

void CMonsterMgr::AddMonster(CGameObject* pGameObject, int iTriggerID)
{
	CMonster* pMonster = dynamic_cast<CMonster*>(pGameObject);
	
	if (pMonster)
	{
		m_mapMonsterGroups[iTriggerID].push_back(pMonster);
	}

	return;
}

void CMonsterMgr::Clear()
{
}

void CMonsterMgr::Free()
{
}
