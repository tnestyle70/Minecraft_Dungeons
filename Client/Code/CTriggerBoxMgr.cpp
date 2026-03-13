#include "pch.h"
#include "CIronBarMgr.h"
#include "CMonsterMgr.h"
#include "CTriggerBoxMgr.h"
#include "CManagement.h"

IMPLEMENT_SINGLETON(CTriggerBoxMgr)

CTriggerBoxMgr::CTriggerBoxMgr()
{
}

CTriggerBoxMgr::~CTriggerBoxMgr()
{
}

HRESULT CTriggerBoxMgr::Ready_TriggerBox()
{
	return S_OK;
}

void CTriggerBoxMgr::Update(const _float& fTimeDelta)
{
	//Trigger 박스 순회하면서 PlayerCollider와 Block들의 Collider와 충돌 처리를 해서 판단
	CComponent* pComponent = CManagement::GetInstance()->Get_Component(
		ID_DYNAMIC, L"GameLogic_Layer", L"Player", L"Com_Collider");
	CCollider* pPlayerCollider = dynamic_cast<CCollider*>(pComponent);
	
	if (!pPlayerCollider)
	{
		MSG_BOX("There is no Player Collider!");
	}

	for (auto& pTriggerBox : m_vecTriggerBox)
	{
		//if already triggered, continue
		if (pTriggerBox->IsTriggered())
			continue;
		pTriggerBox->Update_GameObject(fTimeDelta);
	}
}

void CTriggerBoxMgr::LateUpdate(const _float & fTimeDelta)
{
	for (auto& pTriggerBox : m_vecTriggerBox)
	{
		if (pTriggerBox->IsTriggered())
			continue;
		pTriggerBox->LateUpdate_GameObject(fTimeDelta);
	}
}

void CTriggerBoxMgr::Render()
{
	for (auto& pTriggerBox : m_vecTriggerBox)
	{
		if (pTriggerBox->IsTriggered())
			continue;
		pTriggerBox->Render_GameObject();
	}
}

void CTriggerBoxMgr::AddTriggerBox(CTriggerBox* pTriggerBox)
{
	m_vecTriggerBox.push_back(pTriggerBox);
}

void CTriggerBoxMgr::Clear()
{
	for (auto& pTriggerBox : m_vecTriggerBox)
	{
		Safe_Release(pTriggerBox);
	}
	m_vecTriggerBox.clear();
}

void CTriggerBoxMgr::Free()
{
	for (auto& pTriggerBox : m_vecTriggerBox)
	{
		Safe_Release(pTriggerBox);
	}
	m_vecTriggerBox.clear();
}
