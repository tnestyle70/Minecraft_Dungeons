#include "pch.h"
#include "CCrystal.h"
#include "CRenderer.h"

CCrystal::CCrystal(LPDIRECT3DDEVICE9 pGraphicDev)
	: CGameObject(pGraphicDev)
{
}

CCrystal::~CCrystal()
{
}

HRESULT CCrystal::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	return S_OK;
}

_int CCrystal::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_NONALPHA, this);

	return iExit;
}

void CCrystal::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CCrystal::Render_GameObject()
{
	
}

HRESULT CCrystal::Add_Component()
{
	CComponent* pComponent = nullptr;

	m_mapComponent[ID_STATIC].insert({ { L"Com_Buffer", pComponent } });

	return S_OK;
}

CCrystal* CCrystal::Create(LPDIRECT3DDEVICE9 pGraphicDev)
{
	CCrystal* pCrystal = new CCrystal(pGraphicDev);

	if (FAILED(pCrystal->Ready_GameObject()))
	{
		Safe_Release(pCrystal);
		MSG_BOX("Crystal Create Failed");
		return nullptr;
	}

	return pCrystal;
}

void CCrystal::Free()
{
	CGameObject::Free();
}