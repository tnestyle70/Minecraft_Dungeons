#include "pch.h"
#include "CIronBarMgr.h"

CIronBarMgr::CIronBarMgr()
{
}

CIronBarMgr::~CIronBarMgr()
{
	Free();
}

HRESULT CIronBarMgr::Ready_IronBarMgr()
{
	return S_OK;
}

void CIronBarMgr::Update(const _float& fTimeDelta)
{
	for (auto& pIronBar : m_vecIronBars)
	{
		pIronBar->Update_GameObject(fTimeDelta);
	}

	UpdateIronBarAnim();
}

void CIronBarMgr::LateUpdate(const _float & fTimeDelta)
{
	for (auto& pIronBar : m_vecIronBars)
	{
		pIronBar->LateUpdate_GameObject(fTimeDelta);
	}
}

void CIronBarMgr::Render()
{
	for (auto& pIronBar : m_vecIronBars)
	{
		pIronBar->Render_GameObject();
	}
}

void CIronBarMgr::AddIronBar(CIronBar* pIronBar)
{
	m_vecIronBars.push_back(pIronBar);
}

void CIronBarMgr::Clear()
{
	for (auto& pIronBar : m_vecIronBars)
	{
		Safe_Release(pIronBar);
	}
	m_vecIronBars.clear();
}

void CIronBarMgr::UpdateIronBarAnim()
{
	//IronBar Anim Play
	if (!m_bTriggered)
		return;
	
	for (auto& pIronBar : m_vecIronBars)
	{
		if (pIronBar->IsClosed())
		{
			pIronBar->Update_Animation(eIronBarAnimState::MOVE_UP);
		}
		else
		{
			pIronBar->Update_Animation(eIronBarAnimState::MOVE_DOWN);

		}
	}
}

void CIronBarMgr::Free()
{
	for (auto& pIronBar : m_vecIronBars)
	{
		Safe_Release(pIronBar);
	}
	m_vecIronBars.clear();
}
