#pragma once
#include "CMonster.h"
#include "CPlayer.h"
//스테이지별로 트리거 박스 밟으면 생성될 몬스터 목록들 저장해두고,
//트리거 박스 밟을 경우 몬스터들 지정된 위치로 쭉 스폰되도록 설정
//MonsterMgr에서 IronBarMgr의 false 상태로 전환 시키기

struct SpawnGroup
{
	vector<CMonster*> vecMonsters;
	float fSpawnTimer = 0.f;
	float fSpawnDelay = 1.f;
	int iNextSpawnIndex = 0;
	bool bTriggered = false;
	bool bAllSpawned = false;
};

class CMonsterMgr : public CBase
{
	DECLARE_SINGLETON(CMonsterMgr)
private:
	explicit CMonsterMgr();
	virtual ~CMonsterMgr();
public:
	HRESULT Ready_MonsterMgr();
	_int Update(const _float& fTimeDelta);
	void LateUpdate(const _float& fTimeDelta);
	void Render();
private:
	void SpawnMonsters(const _float& fTimeDelta);
public:
	bool IsGroupAllDead(int iTriggerID);
	void SetActiveMonsterGroup(int iTriggerID);
	void SetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer; }
	CPlayer* Get_Player() const { return m_pPlayer; }

private:
	CPlayer* m_pPlayer = nullptr;

public:
	void AddMonster(CGameObject* pGameObject, int iTriggerID);
	void Clear();
private: //트리거 박스 밟았을 경우 생성할 ID별 몬스터 목록
	map<int, SpawnGroup> m_mapMonsterGroups;
	int m_iTriggerID = -1;
private:
	virtual void Free();
};
