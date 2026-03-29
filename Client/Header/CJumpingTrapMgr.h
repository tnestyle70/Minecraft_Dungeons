#pragma once
#include "CJumpingTrap.h"

class CPlayer;

class CJumpingTarpMgr : public CBase
{
	DECLARE_SINGLETON(CJumpingTarpMgr)
private:
	explicit CJumpingTarpMgr();
	virtual ~CJumpingTarpMgr();
public:
	HRESULT Ready_JumpingTrapMgr();
	_int Update(const _float& fTimeDelta);
	void LateUpdate(const _float& fTimeDelta);
	void Render();
public:
	void Activate(int iTriggerID);
public:
	void Add_JumpingTrap(CGameObject* pGameObject, int iTriggerID);
	void Clear();

	void Set_Player(CPlayer* pPlayer);
public:
	bool Is_Active(int iTriggerID);
private://Jumping Trap을 컨테이너로 관리
	map<int, vector<CJumpingTrap*>> m_mapJumpingTrapGroups;
	bool m_bClosed = false;
	//닫혔는지 열렸는지 map으로 관리
	map<int, bool> m_mapClosed;
	CPlayer* m_pPlayer = nullptr;
private:
	virtual void Free();
};
