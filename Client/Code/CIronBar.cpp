#include "pch.h"
#include "CIronBar.h"
#include "CRenderer.h"

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
	if (FAILED(Add_Component()))
		return E_FAIL;

	m_pTransCom->Set_Pos(vPos.x, vPos.y, vPos.z);
	m_pTransCom->m_vScale = { 0.5f, 10.f, 0.5f };

	return S_OK;
}

_int CIronBar::Update_GameObject(const _float& fTimeDelta)
{
	_int iExit = CGameObject::Update_GameObject(fTimeDelta);

	CRenderer::GetInstance()->Add_RenderGroup(RENDER_ALPHA, this);

	return iExit;
}

void CIronBar::LateUpdate_GameObject(const _float& fTimeDelta)
{
	CGameObject::LateUpdate_GameObject(fTimeDelta);
}

void CIronBar::Render_GameObject()
{
	m_pGraphicDev->SetTransform(D3DTS_WORLD, m_pTransCom->Get_World());

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	m_pTextureCom->Set_Texture(0);

	m_pBufferCom->Render_Buffer();

	m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	//디버깅용으로 일단 렌더링
	m_pCollider->Render_Collider();
}

HRESULT CIronBar::Add_Component()
{
	CComponent* pComponent = nullptr;

	pComponent = m_pBufferCom = dynamic_cast<CCubeTex*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_CubeTex"));
	if (!pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Buffer", pComponent });

	// Transform
	pComponent = m_pTransCom = dynamic_cast<CTransform*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

	// Texture
	//pComponent = m_pTextureCom = dynamic_cast<CTexture*>
	//	(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RockTexture"));
	pComponent = m_pTextureCom = dynamic_cast<CTexture*>
		(CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_PlayerTexture"));

	if (nullptr == pComponent)
		return E_FAIL;

	m_mapComponent[ID_STATIC].insert({ L"Com_Texture", pComponent });

	//Collider

	pComponent = m_pCollider = CCollider::Create(m_pGraphicDev,
		_vec3(1.f, 10.f, 1.f),
		_vec3(0.f, 0.f, 0.f));
	if (!pComponent)
		return E_FAIL;
	m_mapComponent[ID_STATIC].insert({ L"Com_Collider", pComponent });

	return S_OK;
}

CIronBar* CIronBar::Create(LPDIRECT3DDEVICE9 pGraphicDev, const _vec3& vPos)
{
	CIronBar* pIronBar = new CIronBar(pGraphicDev);

	if (FAILED(pIronBar->Ready_GameObject(vPos)))
	{
		Safe_Release(pIronBar);
		MSG_BOX("IronBar Create Failed");
		return nullptr;
	}
	return pIronBar;
}

void CIronBar::Free()
{
	CGameObject::Free();
}
