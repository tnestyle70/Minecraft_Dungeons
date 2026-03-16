#include "pch.h"
#include "CUI.h"
#include "CRcTex.h"
#include "CTexture.h"
#include "CTransform.h"
#include "CProtoMgr.h"
#include "CRenderer.h"

CUI::CUI(LPDIRECT3DDEVICE9 pGraphicDev)
	: Engine::CGameObject(pGraphicDev)
	, m_fX(0.f), m_fY(0.f), m_fSizeX(0.f), m_fSizeY(0.f), m_fUIScale(1.f / 3.f), m_bVisible(true)
{
	D3DXMatrixIdentity(&m_matProj);
}

CUI::CUI(const CUI& rhs)
	: Engine::CGameObject(rhs)
	, m_fX(rhs.m_fX), m_fY(rhs.m_fY), m_fSizeX(rhs.m_fSizeX), m_fSizeY(rhs.m_fSizeY)
	, m_fUIScale(rhs.m_fUIScale)
	, m_bVisible(rhs.m_bVisible)
	, m_matProj(rhs.m_matProj)
{
}

CUI::~CUI()
{
}

HRESULT CUI::Ready_GameObject()
{
	if (FAILED(Add_Component()))
		return E_FAIL;

	D3DXMatrixOrthoOffCenterLH(&m_matProj, 0.f, (float)WINCX, (float)WINCY, 0.f, 0.f, 1.f);

	return S_OK;
}

void CUI::Add_Child(CUI* pChild)
{
	if (!pChild) 
		return;
	pChild->Set_Parent(this);
	pChild->Set_Scale(m_fUIScale); // Match parent scale by default
	m_vecChildren.push_back(pChild);
}

_int CUI::Update_GameObject(const _float& fTimeDelta)
{
	if (!m_bVisible) return 0;

	_int iExit = Engine::CGameObject::Update_GameObject(fTimeDelta);

	for (auto& pChild : m_vecChildren)
		pChild->Update_GameObject(fTimeDelta);

	Engine::CRenderer::GetInstance()->Add_RenderGroup(Engine::RENDER_UI, this);

	return iExit;
}

void CUI::LateUpdate_GameObject(const _float& fTimeDelta)
{
	if (!m_bVisible) return;

	Engine::CGameObject::LateUpdate_GameObject(fTimeDelta);

	m_pTransformCom->m_vScale.x = m_fSizeX * m_fUIScale * 0.5f;
	m_pTransformCom->m_vScale.y = -m_fSizeY * m_fUIScale * 0.5f; // Flip Y for Top-Down screen coordinates
	m_pTransformCom->m_vScale.z = 1.f;

	_float fParentX = 0.f;
	_float fParentY = 0.f;

	if (m_pParent)
	{
		fParentX = m_pParent->m_fX;
		fParentY = m_pParent->m_fY;
	}

	m_pTransformCom->m_vInfo[Engine::INFO_POS].x = (fParentX + m_fX) + m_fSizeX * m_fUIScale * 0.5f;
	m_pTransformCom->m_vInfo[Engine::INFO_POS].y = (fParentY + m_fY) + m_fSizeY * m_fUIScale * 0.5f;
	m_pTransformCom->m_vInfo[Engine::INFO_POS].z = 0.f;

	for (auto& pChild : m_vecChildren)
		pChild->LateUpdate_GameObject(fTimeDelta);
}

void CUI::Render_GameObject()
{
	if (!m_bVisible) return;

	if (m_pTextureCom && m_pBufferCom)
	{
		_matrix matOldView, matOldProj;
		m_pGraphicDev->GetTransform(D3DTS_VIEW, &matOldView);
		m_pGraphicDev->GetTransform(D3DTS_PROJECTION, &matOldProj);

		_matrix matView, matProj;
		D3DXMatrixIdentity(&matView);
		D3DXMatrixOrthoOffCenterLH(&matProj, 0.f, (float)WINCX, (float)WINCY, 0.f, 0.f, 1.f);

		m_pGraphicDev->SetTransform(D3DTS_VIEW, &matView);
		m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matProj);

		m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		m_pGraphicDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		m_pGraphicDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		m_pGraphicDev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		m_pGraphicDev->SetRenderState(D3DRS_LIGHTING, FALSE);
		m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

		m_pGraphicDev->SetRenderState(D3DRS_ZENABLE, FALSE);
		m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

		m_pGraphicDev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		m_pGraphicDev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

		_matrix matTex;
		D3DXMatrixScaling(&matTex, m_vUVScale.x, m_vUVScale.y, 1.f);
		matTex._41 = m_vUVOffset.x;
		matTex._42 = m_vUVOffset.y;

		m_pGraphicDev->SetTransform(D3DTS_TEXTURE0, &matTex);
		m_pGraphicDev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);

		m_pTransformCom->Update_Component(0.f);
		
		_matrix matWorld = *m_pTransformCom->Get_World();
		matWorld._41 -= 0.5f;
		matWorld._42 -= 0.5f;

		m_pGraphicDev->SetTransform(D3DTS_WORLD, &matWorld);

		m_pTextureCom->Set_Texture(0);
		m_pBufferCom->Render_Buffer();

		D3DXMatrixIdentity(&matTex);
		m_pGraphicDev->SetTransform(D3DTS_TEXTURE0, &matTex);
		m_pGraphicDev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);

		m_pGraphicDev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		m_pGraphicDev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

		m_pGraphicDev->SetRenderState(D3DRS_ZENABLE, TRUE);
		m_pGraphicDev->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		m_pGraphicDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		m_pGraphicDev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

		m_pGraphicDev->SetTransform(D3DTS_VIEW, &matOldView);
		m_pGraphicDev->SetTransform(D3DTS_PROJECTION, &matOldProj);
	}

	for (auto& pChild : m_vecChildren)
		pChild->Render_GameObject();
}


HRESULT CUI::Add_Component()
{
	Engine::CComponent* pComponent = nullptr;

	pComponent = m_pBufferCom = dynamic_cast<Engine::CRcTex*>(Engine::CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_RcTex"));
	if (nullptr == pComponent) return E_FAIL;
	m_mapComponent[Engine::ID_STATIC].insert({ L"Com_Buffer", pComponent });

	pComponent = m_pTransformCom = dynamic_cast<Engine::CTransform*>(Engine::CProtoMgr::GetInstance()->Clone_Prototype(L"Proto_Transform"));
	if (nullptr == pComponent) return E_FAIL;
	m_mapComponent[Engine::ID_DYNAMIC].insert({ L"Com_Transform", pComponent });

	return S_OK;
}

void CUI::Free()
{
	Engine::CGameObject::Free();
}
