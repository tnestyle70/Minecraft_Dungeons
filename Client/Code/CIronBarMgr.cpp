#include "pch.h"
#include "CIronBarMgr.h"

IMPLEMENT_SINGLETON(CIronBarMgr)

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

void CIronBarMgr::Open()
{
	m_bClosed = false;

	for (auto& pIronBar : m_vecIronBars)
	{
		pIronBar->SetIronBarState(eIronBarState::MOVE_UP);
	}

	return;
}

void CIronBarMgr::Close()
{
	m_bClosed = true;

	for (auto& pIronBar : m_vecIronBars)
	{
		pIronBar->SetIronBarState(eIronBarState::MOVE_DOWN);
	}

	return;
}

void CIronBarMgr::AddIronBar(CGameObject* pGameObject)
{
	CIronBar* pIronBar = dynamic_cast<CIronBar*>(pGameObject);

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

void CIronBarMgr::Free()
{
	for (auto& pIronBar : m_vecIronBars)
	{
		Safe_Release(pIronBar);
	}
	m_vecIronBars.clear();
}
