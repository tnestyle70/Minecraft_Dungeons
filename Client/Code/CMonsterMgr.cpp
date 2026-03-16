#include "pch.h"
#include "CMonsterMgr.h"

IMPLEMENT_SINGLETON(CMonsterMgr)

CMonsterMgr::CMonsterMgr()
{
}

CMonsterMgr::~CMonsterMgr()
{
	Free();
}

HRESULT CMonsterMgr::Ready_MonsterMgr()
{
	return E_NOTIMPL;
}

_int CMonsterMgr::Update(const _float& fTimeDelta)
{
	for (auto& pair : m_mapMonsterGroups)
	{
		SpawnGroup& group = pair.second;

		//활성된 몬스터만 Update
		for (auto& pMonster : group.vecMonsters)
		{
			if (pMonster->IsActive())
				pMonster->Update_GameObject(fTimeDelta);
		}
		//트리거 안 되었거나, 모두 스폰된 그룹은 continue
		if (!group.bTriggered || group.bAllSpawned)
			continue;
		//스폰 타이머 누적
		group.fSpawnTimer += fTimeDelta;
		if (group.fSpawnTimer < group.fSpawnDelay)
			continue;
		group.fSpawnTimer = 0.f;
		//다음 몬스터 스폰
		group.vecMonsters[group.iNextSpawnIndex]->SetActive(true);
		group.iNextSpawnIndex++;
		//그룹 내 몬스터 전부 스폰 완료되었을 경우 allSpawned = true
		if (group.iNextSpawnIndex >= group.vecMonsters.size())
		{
			group.bAllSpawned = true;
		}
	}
	return 0;
}

void CMonsterMgr::LateUpdate(const _float & fTimeDelta)
{
	for (auto& pair : m_mapMonsterGroups)
	{
		for (auto& pMonster : pair.second.vecMonsters)
		{
			if (pMonster->IsActive())
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
		for (auto& pMonster : pair.second.vecMonsters)
		{
			if (pMonster->IsActive())
			{
				pMonster->Render_GameObject();
			}
		}
	}
}

void CMonsterMgr::SpawnMonsters(const _float& fTimeDelta)
{
}

bool CMonsterMgr::IsGroupAllDead(int iTriggerID)
{
	auto iter = m_mapMonsterGroups.find(iTriggerID);
	if (iter == m_mapMonsterGroups.end())
		return false;
	//몬스터 Alive 플래그 사용해서 판단
	for (auto& pMonster : iter->second.vecMonsters)
	{
		if (!pMonster->Is_Dead())
			return false;
	}
	return true;
}

void CMonsterMgr::SetActiveMonsterGroup(int iTriggerID)
{
	auto iter = m_mapMonsterGroups.find(iTriggerID);

	if (iter == m_mapMonsterGroups.end())
		return;

	SpawnGroup& group = iter->second;
	group.bTriggered = true;
	group.iNextSpawnIndex = 0;
	group.fSpawnTimer = group.fSpawnDelay;
}

void CMonsterMgr::AddMonster(CGameObject* pGameObject, int iTriggerID)
{
	CMonster* pMonster = dynamic_cast<CMonster*>(pGameObject);
	if (!pMonster)
		return;

	if (iTriggerID == 0)
		pMonster->SetActive(true);
	m_mapMonsterGroups[iTriggerID].vecMonsters.push_back(pMonster);

	return;
}

void CMonsterMgr::Clear()
{
	for (auto& pair : m_mapMonsterGroups)
	{
		for (auto& pMonster : pair.second.vecMonsters)
			Safe_Release(pMonster);
	}
	m_mapMonsterGroups.clear();
}

void CMonsterMgr::Free()
{
	Clear();
}
