#include "pch.h"
#include "CIronBar.h"

CIronBar::CIronBar(LPDIRECT3DDEVICE9 pGraphicDev)
	:CGameObject(pGraphicDev)
{
}

CIronBar::CIronBar(const CGameObject& rhs)
	:CGameObject(rhs)
{
}

CIronBar::~CIronBar()
{
}

HRESULT CIronBar::Ready_GameObject(const _vec3& vPos)
{
	return E_NOTIMPL;
}

_int CIronBar::Update_GameObject(const _float& fTimeDelta)
{
	return _int();
}

void CIronBar::LateUpdate_GameObject(const _float& fTimeDelta)
{
}

void CIronBar::Render_GameObject()
{
}

HRESULT CIronBar::Add_Component()
{
	return E_NOTIMPL;
}

CIronBar* CIronBar::Create(LPDIRECT3DDEVICE9 pGraphicDev, const _vec3& vPos)
{
	return nullptr;
}

void CIronBar::Free()
{
}
