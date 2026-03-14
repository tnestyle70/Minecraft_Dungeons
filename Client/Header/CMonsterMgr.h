#pragma once
#include "CMonster.h"
//스테이지별로 트리거 박스 밟으면 생성될 몬스터 목록들 저장해두고,
//트리거 박스 밟을 경우 몬스터들 지정된 위치로 쭉 스폰되도록 설정
//MonsterMgr에서 IronBarMgr의 false 상태로 전환 시키기

class CMonsterMgr : public CBase
{
	DECLARE_SINGLETON(CMonsterMgr)
private:
	explicit CMonsterMgr();
	virtual ~CMonsterMgr();
public:
	HRESULT Ready_MonsterMgr();
	void Update(const _float& fTimeDelta);
	void LateUpdate(const _float& fTimeDelta);
	void Render();
private:
	void SpawnMonsters(const _float& fTimeDelta);
public:
	bool IsGroupAllDead(int iTriggerID);
	void SetTriggerID(int iTriggerID) { m_iTriggerID = iTriggerID; }
public:
	void AddMonster(CGameObject* pGameObject, int iTriggerID);
	void Clear();
private: //트리거 박스 밟았을 경우 생성할 ID별 몬스터 목록
	map<int, vector<CMonster*>> m_mapMonsterGroups;
	int m_iTriggerID = 0;
	//스폰 딜레이
	int m_iSpawnDelay = 0.f;
private:
	virtual void Free();
};
