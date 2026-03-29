#include "pch.h"
#include "CJumpingTrapMgr.h"
#include "CPlayer.h"

IMPLEMENT_SINGLETON(CJumpingTarpMgr)

CJumpingTarpMgr::CJumpingTarpMgr()
{
}

CJumpingTarpMgr::~CJumpingTarpMgr()
{
	Free();
}

HRESULT CJumpingTarpMgr::Ready_JumpingTrapMgr()
{
	return S_OK;
}

_int CJumpingTarpMgr::Update(const _float& fTimeDelta)
{
	for (auto& pair : m_mapJumpingTrapGroups)
	{
		for (auto& pJumpingTrap : pair.second)
		{
			pJumpingTrap->Update_GameObject(fTimeDelta);
		}
	}
	return 0;
}

void CJumpingTarpMgr::LateUpdate(const _float& fTimeDelta)
{
	for (auto& pair : m_mapJumpingTrapGroups)
	{
		for (auto& pJumpingTrap : pair.second)
		{
			pJumpingTrap->LateUpdate_GameObject(fTimeDelta);
		}
	}
}

void CJumpingTarpMgr::Render()
{
	for (auto& pair : m_mapJumpingTrapGroups)
	{
		for (auto& pJumpingTrap : pair.second)
		{
			pJumpingTrap->Render_GameObject();
		}
	}
}

void CJumpingTarpMgr::Activate(int iTriggerID)
{
	auto it = m_mapJumpingTrapGroups.find(iTriggerID);
	if (it == m_mapJumpingTrapGroups.end())
		return;
	//애니메이션 재생
	for (auto* pTrap : it->second)
		pTrap->Set_Active(true);
	//플레이어 점프
	m_pPlayer->LaunchByTrap(50.f);
}

void CJumpingTarpMgr::Add_JumpingTrap(CGameObject * pGameObject, int iTriggerID)
{
	CJumpingTrap* pJumpingTrap = dynamic_cast<CJumpingTrap*>(pGameObject);

	if (pJumpingTrap)
	{
		//iTriggerID의 값이 추가되지 않은 상태일 closed false 추가
		if (m_mapClosed.find(iTriggerID) == m_mapClosed.end())
		{
			m_mapClosed[iTriggerID] = false;
		}

		m_mapJumpingTrapGroups[iTriggerID].push_back(pJumpingTrap);
	}
}

void CJumpingTarpMgr::Clear()
{
	for (auto& [id, vecTraps] : m_mapJumpingTrapGroups)
		for (auto* pTrap : vecTraps)
			Safe_Release(pTrap);

	m_mapJumpingTrapGroups.clear();

	m_pPlayer = nullptr;
}

void CJumpingTarpMgr::Set_Player(CPlayer * pPlayer)
{
	m_pPlayer = pPlayer;
}

bool CJumpingTarpMgr::Is_Active(int iTriggerID)
{
	return false;
}

void CJumpingTarpMgr::Free()
{}
