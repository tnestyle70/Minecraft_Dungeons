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

_int CTriggerBoxMgr::Update(const _float& fTimeDelta)
{
	//처음 세팅한 Collider를 개별 TriggerBox에 주입
	if (!m_pPlayerCollider)
	{
		MSG_BOX("There is no Player Collider!");
		return 0;
	}

	for (auto& pTriggerBox : m_vecTriggerBox)
	{
		pTriggerBox->Update_GameObject(fTimeDelta);

		pTriggerBox->CheckCollide(m_pPlayerCollider);
	}

	return 0;
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

void CTriggerBoxMgr::AddTriggerBox(CGameObject* pGameObject)
{
	CTriggerBox* pTriggerBox = dynamic_cast<CTriggerBox*>(pGameObject);

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
