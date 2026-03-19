#include "pch.h"
#include "CInventoryMgr.h"

IMPLEMENT_SINGLETON(CInventoryMgr)

CInventoryMgr::CInventoryMgr()
{
}

CInventoryMgr::~CInventoryMgr()
{
	Free();
}

HRESULT CInventoryMgr::Ready_InventoryMgr(LPDIRECT3DDEVICE9 pGraphicDev)
{
	m_pGraphicDev = pGraphicDev;

	return S_OK;
}

_int CInventoryMgr::Update(const _float& fTimeDelta)
{
	//I키 단발 토클
	static bool bI_Prev = false;
	bool bI_Cur = (GetAsyncKeyState('I') & 0x8000) != 0;
	if (bI_Cur && !bI_Prev)
		m_bActive = !m_bActive;
	
	if (!m_bActive)
		return 0;

	

	return 0;
}

void CInventoryMgr::LateUpdate(const _float& fTimeDelta)
{

}

void CInventoryMgr::Render()
{

}

void CInventoryMgr::Free()
{
	Safe_Release(m_pGraphicDev);
}
